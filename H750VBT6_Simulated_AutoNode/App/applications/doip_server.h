#ifndef __DOIP_SERVER_H__
#define __DOIP_SERVER_H__

#include <rtthread.h>

#define DOIP_UDP_PORT            13400
#define DOIP_TCP_PORT            13400
#define DOIP_ENTITY_ADDRESS      0x0100
#define DOIP_DEFAULT_TESTER      0x0E00

int doip_server_start(void);

#endif
