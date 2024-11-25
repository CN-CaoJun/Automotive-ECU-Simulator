#ifndef __DOIP_ENTITY_H__
#define __DOIP_ENTITY_H__

#include "doip_timer.h"
#include "doip_event.h"
#include "doip_utils.h"
#include "list.h"

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*------------------------------------------------------------------------------------------------------------*/

#define Generic_Doip_Header_Negative_Ack               0x0000
#define Vehicle_Identify_Request_Message               0x0001
#define Vehicle_Identify_Request_Message_With_EID      0x0002
#define Vehicle_Identify_Request_Message_With_VIN      0x0003
#define Vehicle_Announcememt_Message                   0x0004
#define Vehicle_Identify_Response                      0x0004
#define Routing_Activation_Request                     0x0005
#define Routing_Activation_Response                    0x0006
#define Alive_Check_Request                            0x0007
#define Alive_Check_Response                           0x0008

/* 0x0009 ~ 0x4000 reserved by ISO */

#define Doip_Entity_Status_Request                     0x4001
#define Doip_Entity_Status_Response                    0x4002
#define Diagnotic_Powermode_Information_Request        0x4003
#define Diagnotic_Powermode_Information_Response       0x4004
#define Diagnostic_Message                             0x8001
#define Diagnostic_Positive_ACK                        0x8002
#define Diagnostic_Negative_ACK                        0x8003

/* 0x8004 ~ 0xEFFF reserved by ISO */
/* 0xF000 ~ 0xFFFF reserved by manufactory */

#define A_DoIP_Ctrl                                    2000    /* 2s */
#define A_DoIP_Announce_Wait                           500     /* 500ms */
#define A_DoIP_Announce_Num                            3
#define A_DoIP_Announce_Interval                       500     /* 500ms */
#define A_DoIP_Diagnostic_Message                      2000    /* 2s */
#define T_TCP_General_Inactivity                       300000  /* 5m */
#define T_TCP_Initial_Inactivity                       2000    /* 2s */
#define T_TCP_Alive_Check                              500     /* 500ms */
#define A_Processing_Time                              2000    /* 2s */
#define A_Vehicle_Discovery_Timer                      5000    /* 5s */

#define Header_NACK_Incorrect_Pattern_Format           0x00
#define Header_NACK_Unknow_Payload_type                0x01
#define Header_NACK_Message_Too_Large                  0x02
#define Header_NACK_Out_Of_Memory                      0x03
#define Header_NACK_Invalid_Payload_Len                0x04

#define Diagnostic_NACK_Invalid_Source_addr            0x00
#define Diagnostic_NACK_Unknown_Target_addr            0x01
#define Diagnostic_NACK_Out_Of_Memory                  0x02


/*------------------------------------------------------------------------------------------------------------*/

typedef struct doip_entity doip_entity_t;

enum {
	UNINITIALIZED,
	INITIALIZED,
	ROUTINGACTIVATED,
	FINALIZATION,
};

typedef struct doip_entity doip_entity_t;

typedef struct doip_pdu_t {
	uint8_t protocol;
	uint8_t inverse;
	uint16_t payload_type;
	uint32_t payload_len;
	uint32_t payload_cap;
	uint32_t data_len;
	uint8_t *payload;
} doip_pdu_t;

typedef struct doip_client {
	uint8_t status;
	int handler;
	char client[32];
	uint16_t port;
	uint16_t logic_addr;
	event_io_t watcher;
	timer_t initial_activity_timer;
	timer_t general_activity_timer;
	struct list_head list;
	doip_entity_t *doip_entity;
} doip_client_t;

typedef struct doip_server {
	uint8_t status;
	int handler;
	char addr[32];
	uint16_t port;
	event_io_t watcher;
	uint16_t client_nums;
#define MAX_DOIP_PDU_SIZE  (0x4000)
	doip_pdu_t doip_pdu;
	struct sockaddr_in broadcast; /* for udp server */
	struct sockaddr_in target;
	struct list_head head;
	doip_entity_t *doip_entity;
} doip_server_t;

struct doip_entity {
	char vin[20];
	uint8_t eid[6];
	uint8_t gid[6];
	void *userdata;
	uint16_t logic_addr;
	uint16_t func_addr;
	uint16_t *white_list;
	uint16_t white_list_count;
	double initial_activity_time;
	double general_activity_time;
	int announce_wait_time;
	int announce_count;
	double announce_internal;
    timer_t announce_timer;
	doip_server_t tcp_server;
	doip_server_t udp_server;
};

/*------------------------------------------------------------------------------------------------------------*/

doip_entity_t *doip_entity_alloc();

int doip_entity_start(doip_entity_t *doip_entity);

void doip_entity_set_userdata(doip_entity_t *doip_entity, void *userdata);

void *doip_entity_userdata(doip_entity_t *doip_entity);

void doip_entity_set_initial_activity_time(doip_entity_t *doip_entity, int time);

void doip_entity_set_general_activity_time(doip_entity_t *doip_entity, int time);

void doip_entity_set_announce_wait_time(doip_entity_t *doip_entity, int time);

void doip_entity_set_announce_count(doip_entity_t *doip_entity, int count);

void doip_entity_set_announce_internal(doip_entity_t *doip_entity, int internal);

void doip_entity_set_tcp_server(doip_entity_t *doip_entity, const char *addr, unsigned short port);

void doip_entity_set_udp_server(doip_entity_t *doip_entity, const char *addr, unsigned short port);

void doip_entity_set_logic_addr(doip_entity_t *doip_entity, unsigned short addr);

void doip_entity_set_func_addr(doip_entity_t *doip_entity, unsigned short addr);

void doip_entity_set_white_list(doip_entity_t *doip_entity, unsigned short *addr, int count);

void doip_entity_set_vin(doip_entity_t *doip_entity, const char *vin, int len);

void doip_entity_set_eid(doip_entity_t *doip_entity, unsigned char eid[6]);

void doip_entity_set_gid(doip_entity_t *doip_entity, unsigned char gid[6]);

extern void update_doip_header_len(uint8_t *data, int len, uint32_t payload_len);

extern int assemble_doip_header(uint8_t *data, int len, uint16_t payload_type, uint32_t payload_len);

extern int disassemble_doip_header(uint8_t *data, uint32_t len, doip_pdu_t *doip_pdu);

/*------------------------------------------------------------------------------------------------------------*/

#endif
