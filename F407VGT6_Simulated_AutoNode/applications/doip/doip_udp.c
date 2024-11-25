#include "doip_udp.h"
#include "doip_entity.h"
#include "doip_stream.h"

static void udp_read_cb(void *data);
static int udp_doip_header_verify(doip_pdu_t *doip_pdu, int *errcode);
static ssize_t udp_send_generic_header_nack(doip_entity_t *doip_entity, int nack);
static void start_vehicle_identify_announce_timer(doip_entity_t *doip_entity);
static void vehicle_identify_announce_timer_callback(union sigval val);
static int vehicle_identify_announce(doip_entity_t *doip_entity);
static int vehicle_identify_respon(doip_entity_t *doip_entity);
static int vehicle_identify_respon_with_eid(doip_entity_t *doip_entity);
static int vehicle_identify_respon_with_vin(doip_entity_t *doip_entity);
static int doip_entity_status_respon(doip_entity_t *doip_entity);
static int diagnostic_powermode_information_respon(doip_entity_t *doip_entity);
static ssize_t udp_server_send(doip_entity_t *doip_entity, uint8_t *data, int len);


int udp_server_init(doip_entity_t *doip_entity)
{
	int fd;
	int broadcast = 1;
	struct sockaddr_in server;

	if (!doip_entity) {
		return -1;
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	/* set nonblock */
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

	/* enable broadcast */
	setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (uint8_t *)&broadcast, sizeof(broadcast));

	memset(&server, 0x00, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(doip_entity->udp_server.port);
	inet_pton(AF_INET, doip_entity->udp_server.addr, &server.sin_addr);

	if (bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		goto finish;
	}

	doip_entity->udp_server.handler = fd;
	doip_entity->udp_server.status = INITIALIZED;

	/* init broadcast target */
	memset(&doip_entity->udp_server.broadcast, 0x00, sizeof(doip_entity->udp_server.broadcast));
	doip_entity->udp_server.broadcast.sin_family = AF_INET;
	doip_entity->udp_server.broadcast.sin_port = htons(UDP_DISCOVERY);;
	inet_pton(AF_INET, BROADCAST_ADDR, &doip_entity->udp_server.broadcast.sin_addr);

	return fd;

finish:
	if (fd > 0) {
		close(fd);
		fd = 0;
	}
	return -1;
}

void udp_server_start(doip_entity_t *doip_entity)
{
	event_io_init(&doip_entity->udp_server.watcher, doip_entity->udp_server.handler, (void *)doip_entity, udp_read_cb);

	event_io_start(&doip_entity->udp_server.watcher);

	start_vehicle_identify_announce_timer(doip_entity);
}

static void udp_read_cb(void *data)
{
	int errcode;
	socklen_t socklen;
    event_io_t *watcher = data;
	doip_entity_t *doip_entity = (doip_entity_t *)watcher->data;
	doip_server_t *udp_server = &doip_entity->udp_server;

	socklen = sizeof(udp_server->target);
	ssize_t count = recvfrom(watcher->fd, udp_server->doip_pdu.payload, udp_server->doip_pdu.payload_cap, 0, \
			(struct sockaddr *)&udp_server->target, &socklen);
	if (count == 0) {
		return; /* no data */
	}
	if (count < 0) {
		/* TODO: restart udp server */
		return;
	}

	if (count < 8) {
		return; /* header length */
	}

	disassemble_doip_header(udp_server->doip_pdu.payload, count, &udp_server->doip_pdu);

	if (udp_doip_header_verify(&udp_server->doip_pdu, &errcode) != 0) {
		udp_send_generic_header_nack(doip_entity, errcode);
		return;
	}

	uint16_t payload_type = udp_server->doip_pdu.payload_type;

	logd("payload_type:0x%04x\n", payload_type);

	switch (payload_type) {
		case Generic_Doip_Header_Negative_Ack:
			return; /* ignore */
		case Vehicle_Identify_Request_Message:
			vehicle_identify_respon(doip_entity);
			return;
		case Vehicle_Identify_Request_Message_With_EID:
			vehicle_identify_respon_with_eid(doip_entity);
			return;
		case Vehicle_Identify_Request_Message_With_VIN:
			vehicle_identify_respon_with_vin(doip_entity);
			return;
		case Doip_Entity_Status_Request:
			doip_entity_status_respon(doip_entity);
			return;
		case Diagnotic_Powermode_Information_Request:
			diagnostic_powermode_information_respon(doip_entity);
			return;
		default:
			logd("unknow\n");
	}
}

static int udp_doip_header_verify(doip_pdu_t *doip_pdu, int *errcode)
{
	if (!((doip_pdu->protocol == 0x00 && doip_pdu->inverse == 0xff) || \
		(doip_pdu->protocol == 0x01 && doip_pdu->inverse == 0xfe) || \
		(doip_pdu->protocol == 0x02 && doip_pdu->inverse == 0xfd))) {
		*errcode = Header_NACK_Incorrect_Pattern_Format;
		return -1;
	}

	if (!((doip_pdu->payload_type == Generic_Doip_Header_Negative_Ack) || \
		(doip_pdu->payload_type == Vehicle_Identify_Request_Message) || \
		(doip_pdu->payload_type == Vehicle_Identify_Request_Message_With_EID) || \
		(doip_pdu->payload_type == Vehicle_Identify_Request_Message_With_VIN) || \
		(doip_pdu->payload_type == Doip_Entity_Status_Request) || \
		(doip_pdu->payload_type == Diagnotic_Powermode_Information_Request))) {
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
	if (!((doip_pdu->payload_type == Generic_Doip_Header_Negative_Ack && doip_pdu->data_len == 9) || \
		(doip_pdu->payload_type == Vehicle_Identify_Request_Message && doip_pdu->data_len >= 8) || \
		(doip_pdu->payload_type == Vehicle_Identify_Request_Message_With_EID && doip_pdu->data_len == 14) || \
		(doip_pdu->payload_type == Vehicle_Identify_Request_Message_With_VIN && doip_pdu->data_len == 25) || \
		(doip_pdu->payload_type == Doip_Entity_Status_Request && doip_pdu->data_len == 8) || \
		(doip_pdu->payload_type == Diagnotic_Powermode_Information_Request && doip_pdu->data_len == 8))) {
		*errcode = Header_NACK_Invalid_Payload_Len;
		return -1;
	}

	return 0;
}

static ssize_t udp_send_generic_header_nack(doip_entity_t *doip_entity, int nack)
{
	STREAM_T strm;
	uint8_t buffer[16] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), YX_GetStrmLeftLen(&strm), \
				Generic_Doip_Header_Negative_Ack, 0));
	YX_WriteBYTE_Strm(&strm, nack);
	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	return udp_server_send(doip_entity, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}

static void start_vehicle_identify_announce_timer(doip_entity_t *doip_entity)
{
    timer_init(&doip_entity->announce_timer, vehicle_identify_announce_timer_callback, (void *)doip_entity);
    timer_start(doip_entity->announce_timer, 0, doip_entity->announce_internal/1000);
}

static void vehicle_identify_announce_timer_callback(union sigval val)
{
	static int count = 0;
	doip_entity_t *doip_entity = (doip_entity_t *)val.sival_ptr;

	vehicle_identify_announce(doip_entity);

	if (++count >= doip_entity->announce_count) {
		count = 0;
		timer_stop(doip_entity->announce_timer);
	}
}

static int vehicle_identify_announce(doip_entity_t *doip_entity)
{
	STREAM_T strm;
	uint8_t buffer[64] = {0};
	doip_server_t *udp_server = &doip_entity->udp_server;

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), YX_GetStrmLeftLen(&strm), \
				Vehicle_Announcememt_Message, 0));
	YX_WriteDATA_Strm(&strm, (uint8_t *)doip_entity->vin, 17);
	YX_WriteHWORD_Strm(&strm, doip_entity->logic_addr);
	YX_WriteDATA_Strm(&strm, doip_entity->eid, sizeof(doip_entity->eid));
	YX_WriteDATA_Strm(&strm, doip_entity->gid, sizeof(doip_entity->gid));
	YX_WriteBYTE_Strm(&strm, 0x00);
	YX_WriteBYTE_Strm(&strm, 0x00);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	return sendto(udp_server->handler, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), 0, \
			(struct sockaddr *)&doip_entity->udp_server.broadcast, sizeof(doip_entity->udp_server.broadcast));
}


static int vehicle_identify_respon(doip_entity_t *doip_entity)
{
	STREAM_T strm;
	uint8_t buffer[64] = {0};
	doip_server_t *udp_server = &doip_entity->udp_server;

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmPtr(&strm), YX_GetStrmLeftLen(&strm), \
				Vehicle_Announcememt_Message, 0));
	YX_WriteDATA_Strm(&strm, (uint8_t *)doip_entity->vin, 17);
	YX_WriteHWORD_Strm(&strm, doip_entity->logic_addr);
	YX_WriteDATA_Strm(&strm, doip_entity->eid, sizeof(doip_entity->eid));
	YX_WriteDATA_Strm(&strm, doip_entity->gid, sizeof(doip_entity->gid));
	YX_WriteBYTE_Strm(&strm, 0x00);
	YX_WriteBYTE_Strm(&strm, 0x00);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	int n = sendto(udp_server->handler, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), 0, \
			(struct sockaddr *)&udp_server->target, sizeof(udp_server->target));
	return n;
}

static int vehicle_identify_respon_with_eid(doip_entity_t *doip_entity)
{
	if (memcmp(doip_entity->eid, &doip_entity->udp_server.doip_pdu.payload[8], 6) == 0) {
		return vehicle_identify_respon(doip_entity);
	}

	return 0;
}

static int vehicle_identify_respon_with_vin(doip_entity_t *doip_entity)
{
	if (memcmp(doip_entity->vin, &doip_entity->udp_server.doip_pdu.payload[8], 17) == 0) {
		return vehicle_identify_respon(doip_entity);
	}

	return 0;
}

static int doip_entity_status_respon(doip_entity_t *doip_entity)
{
	STREAM_T strm;
	uint8_t buffer[16] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmStartPtr(&strm), YX_GetStrmLeftLen(&strm), \
				Doip_Entity_Status_Response, 0));
	YX_WriteBYTE_Strm(&strm, 0x01);
	YX_WriteBYTE_Strm(&strm, 2);
	YX_WriteBYTE_Strm(&strm, doip_entity->tcp_server.client_nums);
	YX_WriteLONG_Strm(&strm, doip_entity->tcp_server.doip_pdu.payload_cap);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	return udp_server_send(doip_entity, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}

static int diagnostic_powermode_information_respon(doip_entity_t *doip_entity)
{
	STREAM_T strm;
	uint8_t buffer[16] = {0};

	YX_InitStrm(&strm, buffer, sizeof(buffer));
	YX_MovStrmPtr(&strm, assemble_doip_header(YX_GetStrmStartPtr(&strm), YX_GetStrmLeftLen(&strm), \
				Diagnotic_Powermode_Information_Response, 0));
	YX_WriteBYTE_Strm(&strm, 0x01);

	update_doip_header_len(YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm), YX_GetStrmLen(&strm) - 8);

	return udp_server_send(doip_entity, YX_GetStrmStartPtr(&strm), YX_GetStrmLen(&strm));
}

static ssize_t udp_server_send(doip_entity_t *doip_entity, uint8_t *data, int len)
{
	if (!(doip_entity && doip_entity->udp_server.status == INITIALIZED && doip_entity->udp_server.handler > 0)) {
		return 0;
	}

	return send(doip_entity->udp_server.handler, data, len, 0);
}
