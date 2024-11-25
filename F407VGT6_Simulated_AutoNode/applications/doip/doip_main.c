#include "doip_entity.h"
#include "host_info.h"
#include "doip_utils.h"

#define VIN "00000000000000000"

unsigned short white_list[] ={
	0x0E80
};

void doip_server(void)
{
	
	unsigned char mac_addr[7];
	char ip_addr[16];
	doip_entity_t *doip_entity = doip_entity_alloc();
	static init_flag = 0;
	if (init_flag == 0)
	{
		get_mac_address(mac_addr);
		get_ip_address(ip_addr);
		doip_entity_set_eid(doip_entity, (unsigned char *)mac_addr);
		doip_entity_set_vin(doip_entity, VIN, sizeof(VIN) - 1);
		doip_entity_set_tcp_server(doip_entity, ip_addr, 13400);
		doip_entity_set_udp_server(doip_entity, "0.0.0.0", 13400);
		doip_entity_set_white_list(doip_entity, white_list, ARRAY_SIZEOF(white_list));
		doip_entity_set_logic_addr(doip_entity, 0x1001);
		init_flag = 1;
	}
	
	doip_entity_start(doip_entity);
}

#define THREAD_STACK_SIZE   2048
#define THREAD_PRIORITY     20

void server_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("server", doip_server, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, 20);

    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    }
}


MSH_CMD_EXPORT(server_init, doip_server_thread)
