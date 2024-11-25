#include "doip_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RT_THREAD
#ifdef RT_THREAD
#include <rtthread.h>
#include <lwip/netif.h>

char interface_name[64] = "";

int get_mac_address(unsigned char mac_addr[7]) {
    struct netif *netif = netif_find("e0");

    if (netif == RT_NULL) {
        rt_kprintf("e0 not found\n");
        return -1;
    }

    memcpy(mac_addr, netif->hwaddr, 6);

    return 0;
}

int get_ip_address(char ip_addr[16]) {
    struct netif *netif = netif_find("e0");

    if (netif == RT_NULL) {
        rt_kprintf("e0 not found\n");
        return -1;
    }
    snprintf(ip_addr, 16, "%s", ipaddr_ntoa(&netif->ip_addr));

    return 0;
}

#else

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char interface_name[64] = "";

int get_mac_address(unsigned char mac_addr[7]) {
    struct ifreq ifr;
    int sock;

    if (strlen(interface_name) == 0) {
        logd("Get mac address interface name error");
        return -1;
    }
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        logd("Get mac address opening socket error");
        return -1;
    }
    
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
    
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        logd("Get mac address in ioctl error");
        close(sock);
        return -1;
    }
    
    close(sock);
    memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, 6);
    return 0;
}

int get_ip_address(char ip_addr[16]) {
    struct ifreq ifr;
    int sock;
    
    if (strlen(interface_name) == 0) {
        logd("Get ip address interface name error");
        return -1;
    }
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        logd("Get ip address opening socket error");
        return -1;
    }
    
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
    
    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        logd("Get ip address in ioctl error");
        close(sock);
        return -1;
    }
    
    close(sock);
    snprintf(ip_addr, 16, "%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    return 0;
}

int host_info_init(void) {
    struct if_nameindex *if_nidxs, *intf;
    int ret = -1;

    if_nidxs = if_nameindex();
    if (if_nidxs == NULL) {
        logd("host info init error");
        return ret;
    }

    for (intf = if_nidxs; intf->if_name != NULL && intf->if_index != 0; intf++) {
        if(strstr(intf->if_name, "eth") != NULL) {
            memcpy(interface_name, intf->if_name, strlen(intf->if_name));
            ret = 0;
            break;
        }
    }

    if (intf->if_name == NULL || intf->if_index == 0) {
        logd("host info init error");
    }

    if_freenameindex(if_nidxs);

    return ret;
}

#endif