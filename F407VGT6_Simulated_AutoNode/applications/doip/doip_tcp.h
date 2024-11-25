#ifndef __DOIP_TCP_H__
#define __DOIP_TCP_H__

#include "doip_entity.h"

/*------------------------------------------------------------------------------------------------------------*/

#define TCP_DATA                                       13400

extern int tcp_server_init(doip_entity_t *doip_entity);

extern void tcp_server_start(doip_entity_t *doip_entity);
#endif  /* __DOIP_TCP_H__ */