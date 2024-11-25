#include "doip_tcp.h"
#include "doip_entity.h"
#include "doip_stream.h"

static void accept_cb(void *data);
// static void general_activity_timer_callback(struct ev_loop *loop, ev_timer *w, int e);
static void initial_activity_timer_callback(union sigval val);
static void tcp_read_cb(void *data);
static int tcp_doip_header_verify(doip_pdu_t *doip_pdu, int *errcode);
static ssize_t tcp_send_generic_header_nack(doip_client_t *doip_client, int nack);
static uint8_t tester_logic_addr_verify(doip_entity_t *doip_entity, uint16_t logic_addr);
static void routing_activation_request_handler(doip_client_t *doip_client);
static uint8_t routing_activation_type_verify(uint8_t active_type);
static ssize_t send_routing_activation_negative_respon(doip_client_t *doip_client, int errcode);
static void routing_activation_respon(doip_client_t *doip_client);
static void alive_check_request(doip_client_t *doip_client);
static void disgnostic_message_handler(doip_client_t *doip_client);
static int tcp_doip_diag_msg_verify(doip_client_t *doip_client, uint16_t client_addr, int *errcode);
static void tcp_send_diag_msg_ack(doip_client_t *doip_client, uint16_t type, int errcode);
static ssize_t doip_entity_tcp_send(doip_client_t *client, uint8_t *data, int len);

int tcp_server_init(doip_entity_t *doip_entity)
{
	int fd;
	struct sockaddr_in server;

	if (!doip_entity) {
		return -1;
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	/* set nonblock */
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

	memset(&server, 0x00, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(doip_entity->tcp_server.port);
	inet_pton(AF_INET, doip_entity->tcp_server.addr, &server.sin_addr);

	if (bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		goto finish;
	}

	if (listen(fd, 10) < 0) {
		goto finish;
	}

	doip_entity->tcp_server.handler = fd;
	doip_entity->tcp_server.status = INITIALIZED;
	INIT_LIST_HEAD(&doip_entity->tcp_server.head);
	return fd;

finish:
	if (fd > 0) {
		close(fd);
		fd = 0;
	}
	return -1;
}

void tcp_server_start(doip_entity_t *doip_entity)
{
	if (!doip_entity) {
		return;
	}

    event_io_init(&doip_entity->tcp_server.watcher, doip_entity->tcp_server.handler, (void *)doip_entity, accept_cb);
    event_io_start(&doip_entity->tcp_server.watcher);
}

static void accept_cb(void *data)
{
	socklen_t socklen;
	struct sockaddr_in client;
    event_io_t *watcher = (event_io_t *)data;
	doip_client_t *doip_client = NULL;
	doip_entity_t *doip_entity = (doip_entity_t *)watcher->data;

	socklen = sizeof(client);
	int connfd = accept(watcher->fd, (struct sockaddr *)&client, &socklen);
	if ((connfd < 0) && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		/* no new connection */
		return;
	}

	doip_client = malloc(sizeof(*doip_client));
	doip_assert(!!doip_client, "malloc failed");
	INIT_LIST_HEAD(&doip_client->list);
	doip_client->handler = connfd;
	doip_client->doip_entity = doip_entity;
	doip_client->port = ntohs(client.sin_port);
	inet_ntop(AF_INET, &client.sin_addr, doip_client->client, sizeof(doip_client->client));
	doip_client->status = INITIALIZED;

	doip_client->watcher.data = doip_client;

    event_io_init(&doip_client->watcher, doip_client->handler, (void *)doip_client, tcp_read_cb);
    event_io_start(&doip_client->watcher);

	// ev_timer_init(doip_client->general_activity_timer, general_activity_timer_callback, 0, doip_entity->general_activity_time/1000);
	// ev_timer_start(loop, doip_client->general_activity_timer);
    timer_init(&doip_client->initial_activity_timer, initial_activity_timer_callback, (void *)doip_client);
    timer_start(doip_client->initial_activity_timer, doip_entity->initial_activity_time/1000, 0);

	/* add client to list */
	list_add(&doip_client->list, &doip_entity->tcp_server.head);
	doip_entity->tcp_server.client_nums++;
}

// static void general_activity_timer_callback(struct ev_loop *loop, ev_timer *w, int e)
// {
// 	doip_client_t *doip_client = w->data;

// 	if (doip_client->status != ROUTINGACTIVATED) {
// 		doip_client->status = FINALIZATION;
// 	}
// }

static void initial_activity_timer_callback(union sigval val)
{
	doip_client_t *doip_client = (doip_client_t *)val.sival_ptr;

	if (doip_client->status != ROUTINGACTIVATED) {
		doip_client->status = FINALIZATION;
	}
}

static void tcp_read_cb(void *data)
{
	int errcode = 0;
	ssize_t count = 0, total = 0;
    event_io_t *watcher = (event_io_t *)data;
    doip_client_t *doip_client = (doip_client_t *)watcher->data;
	doip_entity_t *doip_entity = doip_client->doip_entity;
	doip_server_t *tcp_server = &doip_entity->tcp_server;
	doip_pdu_t *doip_pdu = &doip_entity->tcp_server.doip_pdu;

	while ((count = recv(watcher->fd, doip_pdu->payload + total, 8 - total, 0)) > 0) {
		total += count;
		if (total == 0x08) {
			break;
		}
	}

	if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return; /* no data avaliable */
	}

	if (count <= 0) { /* peer disconnected or error */
		doip_client->status = FINALIZATION;
		return;
	}

	if (total != 8) { /* not valid header */
		return;
	}

	disassemble_doip_header(doip_pdu->payload, total, &tcp_server->doip_pdu);

	logd("payload_type:0x%04x\n", tcp_server->doip_pdu.payload_type);

	if (tcp_doip_header_verify(&tcp_server->doip_pdu, &errcode)) {
		tcp_send_generic_header_nack(doip_client, errcode);
		if (errcode == Header_NACK_Incorrect_Pattern_Format) {
			doip_client->status = FINALIZATION;
			return;
		}
	}

	while ((count = recv(watcher->fd, doip_pdu->payload + total, doip_pdu->payload_len, 0)) > 0) {
		total += count;
		if (total == tcp_server->doip_pdu.payload_len + 8) {
			break;
		}
	}

	if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return; /* no data avaliable */
	}

	if (count <= 0) { /* peer disconnected or error */
		doip_client->status = FINALIZATION;
		return;
	}

	switch (doip_entity->tcp_server.doip_pdu.payload_type) {
		case Routing_Activation_Request:
			routing_activation_request_handler(doip_client);
			return;
		case Alive_Check_Request:
			alive_check_request(doip_client);
			return;
		case Diagnostic_Message:
			disgnostic_message_handler(doip_client);
			return;
		default:
			logd("unknow payload_type\n");
			return;
	}
}

static int tcp_doip_header_verify(doip_pdu_t *doip_pdu, int *errcode)
{
	if (!((doip_pdu->protocol == 0x00 && doip_pdu->inverse == 0xff) || \
		(doip_pdu->protocol == 0x01 && doip_pdu->inverse == 0xfe) || \
		(doip_pdu->protocol == 0x02 && doip_pdu->inverse == 0xfd))) {
		*errcode = Header_NACK_Incorrect_Pattern_Format;
		return -1;
	}

	if (!((doip_pdu->payload_type == Routing_Activation_Request) || \
		(doip_pdu->payload_type == Alive_Check_Request) || \
		(doip_pdu->payload_type == Diagnostic_Message))) {
		*errcode = Header_NACK_Unknow_Payload_type;
		return -1;
	}

	/* just for testing */
	if (doip_pdu->data_len > MAX_DOIP_PDU_SIZE/2) {
		*errcode = Header_NACK_Message_Too_Large;
		return -1;
	}

	/* just for testing */
	if (doip_pdu->data_len > MAX_DOIP_PDU_SIZE * 2/3) {
		*errcode = Header_NACK_Out_Of_Memory;
		return -1;
	}

	/* fixed length, min length, max length */
	if (!((doip_pdu->payload_type == Routing_Activation_Request && doip_pdu->data_len == 8) || \
		(doip_pdu->payload_type == Alive_Check_Request && doip_pdu->data_len == 8) || \
		(doip_pdu->payload_type == Diagnostic_Message && doip_pdu->data_len == 8))) {
		*errcode = Header_NACK_Invalid_Payload_Len;
		return -1;
	}

	return 0;
}

static ssize_t tcp_send_generic_header_nack(doip_client_t *doip_client, int nack)
{
	STREAM_T strm;
	uint8_t buffer[16] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), YX_GetStrmLeftLen(&strm), \
				Generic_Doip_Header_Negative_Ack, 0));
	YX_WriteBYTE_Strm(&strm, nack);
	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	return doip_entity_tcp_send(doip_client, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}

static uint8_t tester_logic_addr_verify(doip_entity_t *doip_entity, uint16_t logic_addr)
{
	for (int i = 0; i < doip_entity->white_list_count; i++) {
		if (doip_entity->white_list[i] == logic_addr) {
			return 1;
		}
	}

	return 0;
}

static void routing_activation_request_handler(doip_client_t *doip_client)
{
	STREAM_T strm;
	uint8_t active_type;
	doip_entity_t *doip_entity = doip_client->doip_entity;
	doip_pdu_t *doip_pdu = &doip_entity->tcp_server.doip_pdu;

	YX_InitStrm(&strm, &doip_pdu->payload[doip_pdu->data_len], doip_pdu->payload_len);
	doip_client->logic_addr = YX_ReadHWORD_Strm(&strm);
	active_type = YX_ReadBYTE_Strm(&strm);

	if (!tester_logic_addr_verify(doip_entity, doip_client->logic_addr)) {
		send_routing_activation_negative_respon(doip_client, 0x00);
		doip_client->status = FINALIZATION;
		return;
	}

	if (!routing_activation_type_verify(active_type)) {
		send_routing_activation_negative_respon(doip_client, 0x06);
		doip_client->status = FINALIZATION;
		return;
	}

	routing_activation_respon(doip_client);
	doip_client->status = ROUTINGACTIVATED;
}

static uint8_t routing_activation_type_verify(uint8_t active_type)
{
	return (active_type == 0x00 || active_type == 0x01);
}

static ssize_t send_routing_activation_negative_respon(doip_client_t *doip_client, int errcode)
{
	STREAM_T strm;
	uint8_t buffer[32] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), YX_GetStrmLeftLen(&strm), Routing_Activation_Response, 0));
	YX_WriteHWORD_Strm(&strm, doip_client->logic_addr);
	YX_WriteHWORD_Strm(&strm, doip_client->doip_entity->logic_addr);
	YX_WriteBYTE_Strm(&strm, errcode);
	YX_WriteLONG_Strm(&strm, 0xffffffff);
	YX_WriteLONG_Strm(&strm, 0xffffffff);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	return doip_entity_tcp_send(doip_client, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}

static void routing_activation_respon(doip_client_t *doip_client)
{
	STREAM_T strm;
	doip_entity_t *doip_entity = doip_client->doip_entity;
	// doip_server_t *tcp_server = &doip_client->doip_entity->tcp_server;

	uint8_t buffer[32] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), \
				YX_GetStrmLeftLen(&strm), Routing_Activation_Response, 0));
	YX_WriteHWORD_Strm(&strm, doip_client->logic_addr);
	YX_WriteHWORD_Strm(&strm, doip_entity->logic_addr);
	YX_WriteBYTE_Strm(&strm, 0x10);
	YX_WriteLONG_Strm(&strm, 0x00000000);
	// YX_WriteLONG_Strm(&strm, 0xffffffff);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	doip_entity_tcp_send(doip_client, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}


static void alive_check_request(doip_client_t *doip_client)
{

}

static void disgnostic_message_handler(doip_client_t *doip_client)
{
	int errcode = 0;
	doip_pdu_t *doip_pdu = &doip_client->doip_entity->tcp_server.doip_pdu;
	STREAM_T strm;
	uint16_t target_addr;
	uint16_t client_addr;

	YX_InitStrm(&strm, &doip_pdu->payload[doip_pdu->data_len], doip_pdu->payload_len);
	client_addr = YX_ReadHWORD_Strm(&strm);
	target_addr = YX_ReadHWORD_Strm(&strm);
	if (tcp_doip_diag_msg_verify(doip_client, client_addr, &errcode)) {
		tcp_send_diag_msg_ack(doip_client, Diagnostic_Negative_ACK, errcode);
	} else {
		tcp_send_diag_msg_ack(doip_client, Diagnostic_Positive_ACK, errcode);
	}
	logd("target addr = 0x%x\n", target_addr);
	doip_printf_hex(&doip_pdu->payload[doip_pdu->data_len+4], doip_pdu->payload_len-4);
	// doip_send_to_can(target_addr, &doip_pdu->payload[doip_pdu->data_len+4], doip_pdu->payload_len-4);
}

static int tcp_doip_diag_msg_verify(doip_client_t *doip_client, uint16_t client_addr, int *errcode)
{
	if(!tester_logic_addr_verify(doip_client->doip_entity, client_addr)) {
		*errcode = Diagnostic_NACK_Invalid_Source_addr;
		return -1;
	}

	if (client_addr != doip_client->logic_addr) {
		*errcode = Diagnostic_NACK_Invalid_Source_addr;
		return -1;
	}

	return 0;
}

static void tcp_send_diag_msg_ack(doip_client_t *doip_client, uint16_t type, int errcode)
{
	STREAM_T strm;
	doip_entity_t *doip_entity = doip_client->doip_entity;
	uint8_t buffer[32] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), \
				YX_GetStrmLeftLen(&strm), type, 0));
	YX_WriteHWORD_Strm(&strm, doip_entity->logic_addr);
	YX_WriteHWORD_Strm(&strm, doip_client->logic_addr);
	YX_WriteBYTE_Strm(&strm, (uint8_t)errcode);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	doip_entity_tcp_send(doip_client, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}

static ssize_t doip_entity_tcp_send(doip_client_t *client, uint8_t *data, int len)
{
	ssize_t count = 0, total = 0;

	if (!(client && data && client->handler > 0 && len > 0)) {
		return 0;
	}

	while ((count = send(client->handler, data + total, len - total, 0)) > 0) {
		total += count;
		if (total == count) {
			break;
		}
	}

	return total;
}