#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stdint.h"
#include "time.h"

#include "isotp.h"
#include "isotp_node.h"

#include <rtthread.h>
#include "rtdevice.h"

#define LOG_TAG              "isotp"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define RET_OK 0
#define RET_NOK 1
#define RET_ERR 2
#define RET_NO_DATA 3

#define FRAMESDUMAX 4095
#define CAN_REQUEST_ID 0x731
#define CAN_RESPONSE_ID 0x739
/* Alloc IsoTpLink statically in RAM */
static IsoTpLink g_link;
static IsoTpLink g_doip_link;

/* Alloc send and receive buffer statically in RAM */
static uint8_t g_isotpRecvBuf[FRAMESDUMAX];
static uint8_t g_isotpSendBuf[FRAMESDUMAX];
static uint8_t g_doipRecvBuf[FRAMESDUMAX];
static uint8_t g_doipSendBuf[FRAMESDUMAX];
static uint8_t g_doipOutBuf[FRAMESDUMAX];

static rt_mutex_t g_doip_lock;
static void (*g_doip_rx_callback)(const uint8_t *data, uint16_t len);
static rt_bool_t g_isotp_started;

#define CAN_DEV_NAME "fdcan1"

static struct rt_semaphore tprx_sem;
static rt_device_t can_dev;

static uint8_t isfdcan1open = 0;
static uint8_t isfdcan2open = 0;

static uint8_t sendbuf[100] = {0x00};

#define DEVICEOPEN 1
#define DEVICECLOSE 0
static rt_uint8_t dlc_to_length(uint8_t dlc)
{
    static const rt_uint8_t dlc_to_len_table[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

    if (dlc > 15)
    {
        return 0;
    }

    return dlc_to_len_table[dlc];
}
uint8_t ret_isfdcan1open()
{
    return isfdcan1open;
}
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&tprx_sem);
    return RT_EOK;
}

uint8_t payload[4095] = {0};
uint16_t payload_size = 0;
uint16_t out_size = 0;
static void cantp_rx_thread(void *parameter)
{
    int i;
    int ret;
    rt_size_t read_size;
    struct rt_can_msg rxmsg = {0};

    rt_device_set_rx_indicate(can_dev, can_rx_call);

    while (1)
    {
        rt_sem_take(&tprx_sem, rt_tick_from_millisecond(5));

        if (isfdcan1open == DEVICEOPEN)
        {
            do
            {
                rxmsg.hdr_index = -1;
                read_size = rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));
                if (read_size != sizeof(rxmsg))
                {
                    break;
                }

                LOG_D("RX ID-> %X ", rxmsg.id);
                for (i = 0; i < dlc_to_length(rxmsg.len); i++)
                {
                    LOG_D("%02x", rxmsg.data[i]);
                }

                if (CAN_REQUEST_ID == rxmsg.id)
                {
                    isotp_on_can_message(&g_link, rxmsg.data, dlc_to_length(rxmsg.len));
                }
                else if (CAN_RESPONSE_ID == rxmsg.id && g_doip_lock != RT_NULL)
                {
                    rt_mutex_take(g_doip_lock, RT_WAITING_FOREVER);
                    isotp_on_can_message(&g_doip_link, rxmsg.data, dlc_to_length(rxmsg.len));
                    rt_mutex_release(g_doip_lock);
                }
            } while (1);

            isotp_poll(&g_link);
            payload_size = sizeof(payload);
            ret = isotp_receive(&g_link, payload, payload_size, &out_size);
            if (ISOTP_RET_OK == ret)
            {
                LOG_D("Received %d bytes !!! ",out_size);
                uint16_t send_size = ((payload[0] << 8) | payload[1]);
                LOG_D("Will Send  %d bytes !!! ",send_size);
                ret = isotp_send(&g_link, sendbuf, send_size);
            }
            else
            {
                // LOG_D("Received not complete!!!");
            }

            if (g_doip_lock != RT_NULL)
            {
                rt_mutex_take(g_doip_lock, RT_WAITING_FOREVER);
                isotp_poll(&g_doip_link);
                ret = isotp_receive(&g_doip_link, g_doipOutBuf,
                                    sizeof(g_doipOutBuf), &out_size);
                if (ISOTP_RET_OK == ret && g_doip_rx_callback != RT_NULL)
                {
                    g_doip_rx_callback(g_doipOutBuf, out_size);
                }
                rt_mutex_release(g_doip_lock);
            }
        }
    }
}

int rtcan_isotp_route_send(const uint8_t *data, uint16_t len)
{
    int result;

    if (g_doip_lock == RT_NULL || len == 0 || len > sizeof(g_doipSendBuf))
    {
        return -RT_EINVAL;
    }

    rt_mutex_take(g_doip_lock, RT_WAITING_FOREVER);
    result = isotp_send(&g_doip_link, data, len);
    rt_mutex_release(g_doip_lock);

    return result == ISOTP_RET_OK ? RT_EOK : -RT_ERROR;
}

void rtcan_isotp_set_rx_callback(void (*callback)(const uint8_t *data, uint16_t len))
{
    g_doip_rx_callback = callback;
}

int rt_can_send(const uint32_t arbitration_id,
                const uint8_t *data, const uint8_t DLC)
{
    struct rt_can_msg msg = {0};
    rt_err_t res = RT_ERROR;
    rt_size_t size;

    if ((isfdcan1open == DEVICEOPEN) && (can_dev != RT_NULL))
    {
        msg.id = arbitration_id;
        msg.ide = RT_CAN_STDID;
        msg.rtr = RT_CAN_DTR;
        msg.len = DLC;

        memcpy(msg.data, data, DLC > sizeof(msg.data) ? sizeof(msg.data) : DLC);

        size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
        if (size == 0)
        {
            LOG_D("can dev write data failed!");
            res = RT_ERROR;
        }
        else
        {
            LOG_D("TX ID-> %X ", msg.id);
            // LOG_D("Payload ->");
            for (int i = 0; i < dlc_to_length(msg.len); i++)
            {
                LOG_D("%02x", msg.data[i]);
            }
            res = RT_EOK;
        }
    }

    return res;
}

int cantp_tx_thread(void)
{

    while (1)
    {
        if (isfdcan1open == DEVICEOPEN)
        {
            // isotp_poll(&g_link);
            // rt_thread_mdelay(5);
        }
    }

    return 0;
}

int rtcan_node_start()
{
    rt_err_t res;
    rt_thread_t thread;
    char can_name[RT_NAME_MAX];

    if (g_isotp_started)
    {
        return RT_EOK;
    }

    isotp_init_link(&g_link, CAN_RESPONSE_ID,
                    g_isotpSendBuf, sizeof(g_isotpSendBuf),
                    g_isotpRecvBuf, sizeof(g_isotpRecvBuf));
    isotp_init_link(&g_doip_link, CAN_REQUEST_ID,
                    g_doipSendBuf, sizeof(g_doipSendBuf),
                    g_doipRecvBuf, sizeof(g_doipRecvBuf));
    isotp_set_flow_control_id(&g_link, CAN_RESPONSE_ID);
    isotp_set_flow_control_id(&g_doip_link, CAN_RESPONSE_ID);

    g_doip_lock = rt_mutex_create("doipisotp", RT_IPC_FLAG_PRIO);
    if (g_doip_lock == RT_NULL)
    {
        return -RT_ERROR;
    }

    for (int i = 0; i < 100; i++)
    {
        sendbuf[i] = i + 1;
    }

    rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);

    can_dev = rt_device_find(can_name);

    if (!can_dev)
    {
        LOG_D("find %s failed!", can_name);
        return RT_ERROR;
    }
    else
    {
        LOG_D("find %s success!", can_name);
    }

    rt_sem_init(&tprx_sem, "tprx_sem", 0, RT_IPC_FLAG_FIFO);
    res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);
    if (res == RT_EOK)
    {
        isfdcan1open = DEVICEOPEN;
    }

    thread = rt_thread_create("cantp_rx", cantp_rx_thread, RT_NULL, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
        LOG_D("create cantp_rx thread success!");
        g_isotp_started = RT_TRUE;
    }
    else
    {
        LOG_D("create cantp_rx thread failed!");
    }

    return res;
}

MSH_CMD_EXPORT(rtcan_node_start, can device sample);
