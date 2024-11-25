#ifndef __HOST_INFO_H__
#define __HOST_INFO_H__

extern int host_info_init(void);
extern int get_mac_address(unsigned char mac_addr[7]);
extern int get_ip_address(char ip_addr[16]);
#endif  /* __HOST_INFO_H__ */