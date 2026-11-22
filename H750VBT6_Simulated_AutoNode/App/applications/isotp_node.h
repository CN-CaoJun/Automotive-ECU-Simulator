#ifndef __ISOTP_NODE_H__
#define __ISOTP_NODE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int rtcan_node_start(void);
int rt_can_send(const uint32_t arbitration_id,
                const uint8_t *data,
                const uint8_t DLC);
int rtcan_isotp_route_send(const uint8_t *data, uint16_t len);
void rtcan_isotp_set_rx_callback(void (*callback)(const uint8_t *data, uint16_t len));

#ifdef __cplusplus
}
#endif

#endif
