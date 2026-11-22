#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>

#include "doip_server.h"
#include "isotp_node.h"

#define DBG_TAG "doip"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define DOIP_VERSION                 0x02
#define DOIP_INVERSE_VERSION         0xFD
#define DOIP_HEADER_LEN              8
#define DOIP_MAX_PAYLOAD             2048

#define DOIP_NACK_HEADER             0x0000
#define DOIP_VEHICLE_ID_REQUEST      0x0001
#define DOIP_VEHICLE_ID_RESPONSE     0x0004
#define DOIP_ROUTING_ACT_REQUEST     0x0005
#define DOIP_ROUTING_ACT_RESPONSE    0x0006
#define DOIP_ALIVE_CHECK_REQUEST     0x0007
#define DOIP_ALIVE_CHECK_RESPONSE    0x0008
#define DOIP_ENTITY_STATUS_REQUEST   0x4001
#define DOIP_ENTITY_STATUS_RESPONSE  0x4002
#define DOIP_POWER_MODE_REQUEST      0x4003
#define DOIP_POWER_MODE_RESPONSE     0x4004
#define DOIP_DIAG_MESSAGE            0x8001
#define DOIP_DIAG_ACK                0x8002
#define DOIP_DIAG_NACK               0x8003

#define DOIP_NACK_INCORRECT_PATTERN  0x00
#define DOIP_NACK_UNKNOWN_TYPE       0x01
#define DOIP_NACK_TOO_LARGE          0x02
#define DOIP_ROUTING_ACTIVATED       0x10
#define DOIP_DIAG_ACK_OK             0x00
#define DOIP_DIAG_NACK_INVALID_SRC   0x02
#define DOIP_DIAG_NACK_UNKNOWN_TGT   0x03
#define DOIP_DIAG_NACK_UNREACHABLE   0x06

static int g_udp_socket = -1;
static int g_tcp_socket = -1;
static int g_client_socket = -1;
static rt_mutex_t g_client_lock;
static rt_bool_t g_started;
static rt_bool_t g_routing_activated;
static rt_uint16_t g_tester_address = DOIP_DEFAULT_TESTER;

static volatile rt_bool_t g_udp_ready;
static volatile rt_bool_t g_tcp_ready;
static volatile rt_uint32_t g_udp_rx_count;
static volatile rt_uint32_t g_tcp_rx_count;
static volatile rt_uint32_t g_tx_count;
static volatile rt_uint32_t g_can_tx_count;
static volatile rt_uint32_t g_can_rx_count;
static rt_uint8_t g_can_response[DOIP_MAX_PAYLOAD + 4];

static rt_uint16_t get_be16(const rt_uint8_t *data)
{
    return ((rt_uint16_t)data[0] << 8) | data[1];
}

static rt_uint32_t get_be32(const rt_uint8_t *data)
{
    return ((rt_uint32_t)data[0] << 24) |
           ((rt_uint32_t)data[1] << 16) |
           ((rt_uint32_t)data[2] << 8) |
           data[3];
}

static void put_be16(rt_uint8_t *data, rt_uint16_t value)
{
    data[0] = (rt_uint8_t)(value >> 8);
    data[1] = (rt_uint8_t)value;
}

static void put_be32(rt_uint8_t *data, rt_uint32_t value)
{
    data[0] = (rt_uint8_t)(value >> 24);
    data[1] = (rt_uint8_t)(value >> 16);
    data[2] = (rt_uint8_t)(value >> 8);
    data[3] = (rt_uint8_t)value;
}

static int send_all(int socket, const void *buffer, rt_size_t length)
{
    const rt_uint8_t *cursor = buffer;

    while (length > 0)
    {
        int sent = lwip_send(socket, cursor, length, 0);

        if (sent <= 0)
        {
            return -1;
        }

        cursor += sent;
        length -= sent;
    }

    return 0;
}

static int recv_all(int socket, void *buffer, rt_size_t length)
{
    rt_uint8_t *cursor = buffer;

    while (length > 0)
    {
        int received = lwip_recv(socket, cursor, length, 0);

        if (received <= 0)
        {
            return -1;
        }

        cursor += received;
        length -= received;
    }

    return 0;
}

static int send_doip_frame(int socket, rt_uint16_t payload_type,
                           const rt_uint8_t *payload, rt_uint32_t length)
{
    rt_uint8_t header[DOIP_HEADER_LEN];

    header[0] = DOIP_VERSION;
    header[1] = DOIP_INVERSE_VERSION;
    put_be16(&header[2], payload_type);
    put_be32(&header[4], length);

    if (send_all(socket, header, sizeof(header)) != 0)
    {
        return -1;
    }

    if (length > 0 && send_all(socket, payload, length) != 0)
    {
        return -1;
    }

    g_tx_count++;
    return 0;
}

static int send_doip_datagram(int socket, const struct sockaddr_in *peer,
                              rt_uint16_t payload_type,
                              const rt_uint8_t *payload, rt_uint32_t length)
{
    rt_uint8_t frame[64];

    if (length > sizeof(frame) - DOIP_HEADER_LEN)
    {
        return -1;
    }

    frame[0] = DOIP_VERSION;
    frame[1] = DOIP_INVERSE_VERSION;
    put_be16(&frame[2], payload_type);
    put_be32(&frame[4], length);
    memcpy(&frame[DOIP_HEADER_LEN], payload, length);

    if (lwip_sendto(socket, frame, DOIP_HEADER_LEN + length, 0,
                    (const struct sockaddr *)peer, sizeof(*peer)) < 0)
    {
        return -1;
    }

    g_tx_count++;
    return 0;
}

static void send_client_frame(rt_uint16_t payload_type,
                              const rt_uint8_t *payload, rt_uint32_t length)
{
    rt_mutex_take(g_client_lock, RT_WAITING_FOREVER);
    if (g_client_socket >= 0)
    {
        send_doip_frame(g_client_socket, payload_type, payload, length);
    }
    rt_mutex_release(g_client_lock);
}

static void send_header_nack(int socket, rt_uint8_t code)
{
    (void)socket;
    send_client_frame(DOIP_NACK_HEADER, &code, 1);
}

static void build_vehicle_id_payload(rt_uint8_t payload[33])
{
    static const char vin[] = "H750SIM0000000001";
    rt_uint32_t uid0 = *(volatile rt_uint32_t *)(UID_BASE + 0);
    rt_uint32_t uid1 = *(volatile rt_uint32_t *)(UID_BASE + 4);

    memcpy(payload, vin, 17);
    put_be16(&payload[17], DOIP_ENTITY_ADDRESS);
    payload[19] = (rt_uint8_t)(uid0 >> 24);
    payload[20] = (rt_uint8_t)(uid0 >> 16);
    payload[21] = (rt_uint8_t)(uid0 >> 8);
    payload[22] = (rt_uint8_t)uid0;
    payload[23] = (rt_uint8_t)(uid1 >> 24);
    payload[24] = (rt_uint8_t)(uid1 >> 16);
    memset(&payload[25], 0, 6);
    payload[31] = 0x00;
    payload[32] = 0x00;
}

static void handle_udp_request(int socket, const struct sockaddr_in *peer,
                               const rt_uint8_t *data, rt_uint32_t length)
{
    rt_uint8_t response[33];
    rt_uint16_t payload_type;

    if (length < DOIP_HEADER_LEN ||
        data[0] != DOIP_VERSION ||
        data[1] != DOIP_INVERSE_VERSION)
    {
        return;
    }

    payload_type = get_be16(&data[2]);
    if (payload_type != DOIP_VEHICLE_ID_REQUEST)
    {
        return;
    }

    g_udp_rx_count++;
    build_vehicle_id_payload(response);
    send_doip_datagram(socket, peer, DOIP_VEHICLE_ID_RESPONSE,
                       response, sizeof(response));
}

static void handle_diagnostic_message(const rt_uint8_t *payload, rt_uint32_t length)
{
    rt_uint16_t source_address;
    rt_uint16_t target_address;
    rt_uint8_t ack[5];
    int result;

    if (length < 4)
    {
        send_header_nack(g_client_socket, DOIP_NACK_TOO_LARGE);
        return;
    }

    source_address = get_be16(payload);
    target_address = get_be16(&payload[2]);
    put_be16(&ack[0], source_address);
    put_be16(&ack[2], target_address);

    if (!g_routing_activated || source_address != g_tester_address)
    {
        ack[4] = DOIP_DIAG_NACK_INVALID_SRC;
        send_client_frame(DOIP_DIAG_NACK, ack, sizeof(ack));
        return;
    }

    if (target_address != DOIP_ENTITY_ADDRESS)
    {
        ack[4] = DOIP_DIAG_NACK_UNKNOWN_TGT;
        send_client_frame(DOIP_DIAG_NACK, ack, sizeof(ack));
        return;
    }

    result = rtcan_isotp_route_send(&payload[4], length - 4);
    if (result != RT_EOK)
    {
        ack[4] = DOIP_DIAG_NACK_UNREACHABLE;
        send_client_frame(DOIP_DIAG_NACK, ack, sizeof(ack));
        return;
    }

    g_can_tx_count++;
    ack[4] = DOIP_DIAG_ACK_OK;
    send_client_frame(DOIP_DIAG_ACK, ack, sizeof(ack));
}

static void handle_tcp_message(const rt_uint8_t *data, rt_uint32_t length)
{
    rt_uint16_t payload_type;
    const rt_uint8_t *payload;
    rt_uint32_t payload_length;
    rt_uint8_t response[13];

    if (length < DOIP_HEADER_LEN)
    {
        send_header_nack(g_client_socket, DOIP_NACK_INCORRECT_PATTERN);
        return;
    }

    if (data[0] != DOIP_VERSION || data[1] != DOIP_INVERSE_VERSION)
    {
        send_header_nack(g_client_socket, DOIP_NACK_INCORRECT_PATTERN);
        return;
    }

    payload_type = get_be16(&data[2]);
    payload_length = get_be32(&data[4]);
    if (payload_length != length - DOIP_HEADER_LEN)
    {
        send_header_nack(g_client_socket, DOIP_NACK_TOO_LARGE);
        return;
    }

    payload = &data[DOIP_HEADER_LEN];
    g_tcp_rx_count++;

    switch (payload_type)
    {
    case DOIP_ROUTING_ACT_REQUEST:
        if (payload_length < 2)
        {
            send_header_nack(g_client_socket, DOIP_NACK_TOO_LARGE);
            break;
        }

        g_tester_address = get_be16(payload);
        g_routing_activated = RT_TRUE;
        put_be16(&response[0], g_tester_address);
        put_be16(&response[2], DOIP_ENTITY_ADDRESS);
        response[4] = DOIP_ROUTING_ACTIVATED;
        memset(&response[5], 0, 8);
        send_client_frame(DOIP_ROUTING_ACT_RESPONSE, response, sizeof(response));
        break;

    case DOIP_ALIVE_CHECK_REQUEST:
        put_be16(response, g_tester_address);
        send_client_frame(DOIP_ALIVE_CHECK_RESPONSE, response, 2);
        break;

    case DOIP_ENTITY_STATUS_REQUEST:
        response[0] = 0x00;
        response[1] = 0x01;
        response[2] = 0x01;
        put_be32(&response[3], DOIP_MAX_PAYLOAD);
        send_client_frame(DOIP_ENTITY_STATUS_RESPONSE, response, 7);
        break;

    case DOIP_POWER_MODE_REQUEST:
        response[0] = 0x01;
        send_client_frame(DOIP_POWER_MODE_RESPONSE, response, 1);
        break;

    case DOIP_DIAG_MESSAGE:
        handle_diagnostic_message(payload, payload_length);
        break;

    default:
        send_header_nack(g_client_socket, DOIP_NACK_UNKNOWN_TYPE);
        break;
    }
}

static void doip_tcp_client(int client_socket)
{
    rt_uint8_t header[DOIP_HEADER_LEN];
    rt_uint8_t *frame;
    rt_uint32_t payload_length;
    rt_uint32_t total_length;

    g_routing_activated = RT_FALSE;
    g_tester_address = DOIP_DEFAULT_TESTER;

    while (1)
    {
        if (recv_all(client_socket, header, sizeof(header)) != 0)
        {
            break;
        }

        if (header[0] != DOIP_VERSION ||
            header[1] != DOIP_INVERSE_VERSION)
        {
            send_header_nack(client_socket, DOIP_NACK_INCORRECT_PATTERN);
            break;
        }

        payload_length = get_be32(&header[4]);
        if (payload_length > DOIP_MAX_PAYLOAD)
        {
            send_header_nack(client_socket, DOIP_NACK_TOO_LARGE);
            break;
        }

        total_length = DOIP_HEADER_LEN + payload_length;
        frame = rt_malloc(total_length);
        if (frame == RT_NULL)
        {
            send_header_nack(client_socket, DOIP_NACK_TOO_LARGE);
            break;
        }

        memcpy(frame, header, DOIP_HEADER_LEN);
        if (payload_length > 0)
        {
            if (recv_all(client_socket, &frame[DOIP_HEADER_LEN], payload_length) != 0)
            {
                rt_free(frame);
                break;
            }
        }

        handle_tcp_message(frame, total_length);
        rt_free(frame);
    }

    rt_mutex_take(g_client_lock, RT_WAITING_FOREVER);
    g_client_socket = -1;
    rt_mutex_release(g_client_lock);
    lwip_close(client_socket);
    g_routing_activated = RT_FALSE;
}

static void doip_udp_thread(void *parameter)
{
    struct sockaddr_in address;
    struct sockaddr_in peer;
    socklen_t peer_length;
    int receive_length;
    int reuse = 1;

    g_udp_socket = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (g_udp_socket < 0)
    {
        LOG_E("UDP socket create failed");
        return;
    }

    lwip_setsockopt(g_udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = lwip_htons(DOIP_UDP_PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    if (lwip_bind(g_udp_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        LOG_E("UDP bind failed");
        lwip_close(g_udp_socket);
        return;
    }

    g_udp_ready = RT_TRUE;
    LOG_I("DoIP UDP discovery listening on 13400");

    while (1)
    {
        rt_uint8_t buffer[DOIP_HEADER_LEN];

        peer_length = sizeof(peer);
        receive_length = lwip_recvfrom(g_udp_socket, buffer, sizeof(buffer), 0,
                                       (struct sockaddr *)&peer, &peer_length);
        if (receive_length == sizeof(buffer))
        {
            handle_udp_request(g_udp_socket, &peer, buffer, receive_length);
        }
    }
}

static void doip_tcp_thread(void *parameter)
{
    struct sockaddr_in address;
    struct sockaddr_in peer;
    socklen_t peer_length;
    int client_socket;
    int reuse = 1;

    g_tcp_socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (g_tcp_socket < 0)
    {
        LOG_E("TCP socket create failed");
        return;
    }

    lwip_setsockopt(g_tcp_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = lwip_htons(DOIP_TCP_PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    if (lwip_bind(g_tcp_socket, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        lwip_listen(g_tcp_socket, 1) < 0)
    {
        LOG_E("TCP bind/listen failed");
        lwip_close(g_tcp_socket);
        return;
    }

    g_tcp_ready = RT_TRUE;
    LOG_I("DoIP TCP routing listening on 13400");

    while (1)
    {
        peer_length = sizeof(peer);
        client_socket = lwip_accept(g_tcp_socket, (struct sockaddr *)&peer, &peer_length);
        if (client_socket < 0)
        {
            rt_thread_mdelay(100);
            continue;
        }

        rt_mutex_take(g_client_lock, RT_WAITING_FOREVER);
        g_client_socket = client_socket;
        rt_mutex_release(g_client_lock);
        doip_tcp_client(client_socket);
    }
}

static void doip_can_rx(const rt_uint8_t *data, rt_uint16_t length)
{
    if (!g_routing_activated || length > DOIP_MAX_PAYLOAD)
    {
        return;
    }

    g_can_rx_count++;
    put_be16(&g_can_response[0], DOIP_ENTITY_ADDRESS);
    put_be16(&g_can_response[2], g_tester_address);
    memcpy(&g_can_response[4], data, length);
    send_client_frame(DOIP_DIAG_MESSAGE, g_can_response, length + 4);
}

static int doip_info(void)
{
    rt_kprintf("DoIP UDP: %s, TCP: %s\n",
               g_udp_ready ? "ready" : "down",
               g_tcp_ready ? "ready" : "down");
    rt_kprintf("DoIP routing: %s, tester=0x%04X\n",
               g_routing_activated ? "active" : "inactive",
               g_tester_address);
    rt_kprintf("DoIP counters: udp=%u tcp=%u tx=%u can_tx=%u can_rx=%u\n",
               g_udp_rx_count, g_tcp_rx_count, g_tx_count,
               g_can_tx_count, g_can_rx_count);
    return 0;
}
MSH_CMD_EXPORT(doip_info, show DoIP server state);

int doip_server_start(void)
{
    rt_thread_t thread;

    if (g_started)
    {
        return RT_EOK;
    }

    g_client_lock = rt_mutex_create("doiptx", RT_IPC_FLAG_PRIO);
    if (g_client_lock == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    rtcan_isotp_set_rx_callback(doip_can_rx);

    thread = rt_thread_create("doip_udp", doip_udp_thread, RT_NULL, 4096, 20, 10);
    if (thread == RT_NULL)
    {
        return -RT_ENOMEM;
    }
    rt_thread_startup(thread);

    thread = rt_thread_create("doip_tcp", doip_tcp_thread, RT_NULL, 4096, 21, 10);
    if (thread == RT_NULL)
    {
        return -RT_ENOMEM;
    }
    rt_thread_startup(thread);

    g_started = RT_TRUE;
    return RT_EOK;
}
