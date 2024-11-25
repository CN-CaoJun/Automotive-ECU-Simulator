#ifndef __DOIP_UDP_H__
#define __DOIP_UDP_H__

#include "doip_entity.h"

/*------------------------------------------------------------------------------------------------------------*/

#define BROADCAST_ADDR                                 "255.255.255.255"
#define UDP_DISCOVERY                                  13400

extern int udp_server_init(doip_entity_t *doip_entity);

extern void udp_server_start(doip_entity_t *doip_entity);

#endif  /* __DOIP_UDP_H__ */