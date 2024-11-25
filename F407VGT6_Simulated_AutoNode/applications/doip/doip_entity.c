#include "doip_entity.h"
#include "doip_stream.h"
#include "doip_utils.h"
#include "doip_tcp.h"
#include "doip_udp.h"

#include <errno.h>
#include <string.h>
#include <malloc.h>

void doip_entity_set_userdata(doip_entity_t *doip_entity, void *userdata)
{
	if (doip_entity) {
		doip_entity->userdata = userdata;
	}
}

void *doip_entity_userdata(doip_entity_t *doip_entity)
{
	if (doip_entity) {
		return doip_entity->userdata;
	}
	return NULL;
}

void doip_entity_set_initial_activity_time(doip_entity_t *doip_entity, int time)
{
	if (doip_entity) {
		doip_entity->initial_activity_time = time;
	}
}

void doip_entity_set_general_activity_time(doip_entity_t *doip_entity, int time)
{
	if (doip_entity) {
		doip_entity->general_activity_time = time;
	}
}

void doip_entity_set_announce_wait_time(doip_entity_t *doip_entity, int time)
{
	if (doip_entity) {
		doip_entity->announce_wait_time = time;
	}
}

void doip_entity_set_announce_count(doip_entity_t *doip_entity, int count)
{
	if (doip_entity) {
		doip_entity->announce_count = count;
	}
}

void doip_entity_set_announce_internal(doip_entity_t *doip_entity, int internal)
{
	if (doip_entity) {
		doip_entity->announce_internal = internal;
	}
}

void doip_entity_set_tcp_server(doip_entity_t *doip_entity, const char *addr, unsigned short port)
{
	if (!(doip_entity && addr)) {
		return;
	}

	doip_entity->tcp_server.port = port;
	memcpy(doip_entity->tcp_server.addr, addr, sizeof(doip_entity->tcp_server.addr));
}

void doip_entity_set_udp_server(doip_entity_t *doip_entity, const char *addr, unsigned short port)
{
	if (!(doip_entity && addr)) {
		return;
	}

	doip_entity->udp_server.port = port;
	memcpy(doip_entity->udp_server.addr, addr, sizeof(doip_entity->udp_server.addr));
}

void doip_entity_set_logic_addr(doip_entity_t *doip_entity, unsigned short addr)
{
	if (doip_entity) {
		doip_entity->logic_addr = addr;
	}
}

void doip_entity_set_func_addr(doip_entity_t *doip_entity, unsigned short addr)
{
	if (doip_entity) {
		doip_entity->func_addr = addr;
	}
}

void doip_entity_set_white_list(doip_entity_t *doip_entity, unsigned short *addr, int count)
{
	int i;

	if (doip_entity) {
		doip_entity->white_list_count = count;
		if(doip_entity->white_list != NULL) {
			free(doip_entity->white_list);
		}
		doip_entity->white_list = malloc(count * sizeof(uint16_t));
		for (i = 0; i < count; i++)
			doip_entity->white_list[i] = addr[i];
	}
}

void doip_entity_set_vin(doip_entity_t *doip_entity, const char *vin, int len)
{
	if (doip_entity) {
		memset(doip_entity->vin, 0x00, sizeof(doip_entity->vin));
		memcpy(doip_entity->vin, vin, sizeof(doip_entity->vin));
	}
}

void doip_entity_set_eid(doip_entity_t *doip_entity, unsigned char eid[6])
{
	if (doip_entity) {
		memset(doip_entity->eid, 0x00, sizeof(doip_entity->eid));
		memcpy(doip_entity->eid, eid, sizeof(doip_entity->eid));
	}
}

void doip_entity_set_gid(doip_entity_t *doip_entity, unsigned char gid[6])
{
	if (doip_entity) {
		memset(doip_entity->gid, 0x00, sizeof(doip_entity->gid));
		memcpy(doip_entity->gid, gid, sizeof(doip_entity->gid));
	}
}

void update_doip_header_len(uint8_t *data, int len, uint32_t payload_len)
{
	STREAM_T strm;

	if (len > 8) {
		YX_InitStrm(&strm, data, len);
		YX_MovStrmPtr(&strm, 4);
		YX_WriteLONG_Strm(&strm, payload_len);
	}
}

int assemble_doip_header(uint8_t *data, int len, uint16_t payload_type, uint32_t payload_len)
{
	STREAM_T strm;

	YX_InitStrm(&strm, data, len);
	YX_WriteBYTE_Strm(&strm, 0x02);
	YX_WriteBYTE_Strm(&strm, 0xfd);
	YX_WriteHWORD_Strm(&strm, payload_type);
	YX_WriteLONG_Strm(&strm, payload_len);
	return YX_GetStrmLen(&strm);
}

int disassemble_doip_header(uint8_t *data, uint32_t len, doip_pdu_t *doip_pdu)
{
	STREAM_T strm;

	if (len < 8) {
		return -1;
	}

	YX_InitStrm(&strm, data, len);
	doip_pdu->protocol = YX_ReadBYTE_Strm(&strm);
	doip_pdu->inverse = YX_ReadBYTE_Strm(&strm);
	doip_pdu->payload_type = YX_ReadHWORD_Strm(&strm);
	doip_pdu->payload_len = YX_ReadLONG_Strm(&strm);
	doip_pdu->data_len = len;

	// logd("protocol:0x%02x\n", doip_pdu->protocol);
	// logd("inverse:0x%02x\n", doip_pdu->inverse);
	// logd("payload_type:0x%04x\n", doip_pdu->payload_type);
	// logd("payload_len:0x%x\n\n", doip_pdu->payload_len);

	return YX_GetStrmLen(&strm);
}

static void prepare_connect(doip_entity_t *doip_entity)
{
	if (doip_entity->tcp_server.status == UNINITIALIZED) {
		/* TODO clear all clients */
		if (tcp_server_init(doip_entity) < 0) {
			logd("tcp_server_init failed\n");
			return;
		}
		logd("tcp_server_init\n");
		tcp_server_start(doip_entity);
	}
	if (doip_entity->udp_server.status == UNINITIALIZED) {
		if (udp_server_init(doip_entity) < 0) {
			logd("udp_server_init failed\n");
			return;
		}
		logd("udp_server_init\n");
		udp_server_start(doip_entity);
	}
}

static void doip_client_clean(doip_client_t *doip_client)
{
	// doip_entity_t *doip_entity = doip_client->doip_entity;

	event_io_stop(&doip_client->watcher);
	timer_stop(doip_client->initial_activity_timer);
	// ev_timer_stop(loop, doip_client->general_activity_timer);
	close(doip_client->watcher.fd);
	// free(doip_client->general_activity_timer);
}

static void doip_tcp_server_clean(doip_server_t *tcp_server)
{
	doip_client_t *client, *temp;

	if (tcp_server->client_nums == 0) {
		return;
	}
	list_for_each_entry_safe(client, temp, &tcp_server->head, list) {
		doip_client_clean(client);
		list_del(&client->list);
		free(client);
	}
	tcp_server->client_nums = 0;
}

/* 集中处理需要关闭的客户端,重启服务器本身(服务器出问题的话) */
static void check_connect(doip_entity_t *doip_entity)
{
	doip_client_t *client, *temp;
	doip_server_t *tcp_server = &doip_entity->tcp_server;
	// doip_server_t *udp_server = &doip_entity->udp_server;

	if (tcp_server->status == FINALIZATION) {
		doip_tcp_server_clean(tcp_server);
	}

	if (tcp_server->client_nums == 0) {
		return;
	}

	list_for_each_entry_safe(client, temp, &tcp_server->head, list) {
		if (client->status == FINALIZATION) {
			doip_client_clean(client);
			list_del(&client->list);
			tcp_server->client_nums--;
		}
	}
}

doip_entity_t *doip_entity_alloc()
{
	doip_entity_t *doip_entity = malloc(sizeof(doip_entity_t));

	if (!doip_entity) {
		return NULL;
	}

	memset(doip_entity, 0x00, sizeof(doip_entity_t));
	doip_entity->udp_server.doip_pdu.payload_cap = MAX_DOIP_PDU_SIZE;
	doip_entity->tcp_server.doip_pdu.payload_cap = MAX_DOIP_PDU_SIZE;
	doip_entity->tcp_server.doip_pdu.payload = malloc(doip_entity->tcp_server.doip_pdu.payload_cap);
	doip_entity->udp_server.doip_pdu.payload = malloc(doip_entity->tcp_server.doip_pdu.payload_cap);

	doip_entity->tcp_server.doip_entity = doip_entity;
	doip_entity->udp_server.doip_entity = doip_entity;

	doip_assert(doip_entity->tcp_server.doip_pdu.payload && \
			doip_entity->udp_server.doip_pdu.payload, "malloc failed");

	doip_entity->initial_activity_time = T_TCP_Initial_Inactivity;
	doip_entity->general_activity_time = T_TCP_General_Inactivity;
	doip_entity->announce_count = A_DoIP_Announce_Num;
	doip_entity->announce_internal = A_DoIP_Announce_Interval;
	doip_entity->announce_wait_time = A_DoIP_Announce_Wait;

	return doip_entity;
}

int doip_entity_start(doip_entity_t *doip_entity)
{
	if (!doip_entity) {
		return -1;
	}

	while (1) {
		prepare_connect(doip_entity);
		check_connect(doip_entity);
	}

	return 0;
}
