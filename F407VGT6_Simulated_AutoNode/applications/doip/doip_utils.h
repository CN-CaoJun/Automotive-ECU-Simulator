#ifndef __DOIP_UTILS_H__
#define __DOIP_UTILS_H__

#include <stdint.h>
#include <stdio.h>
#include <rtthread.h>


#define DEBUG

#ifdef DEBUG
#define logd(format, args...) rt_kprintf("[line:%d func:%s]:"format, __LINE__, __FUNCTION__, ##args);
#else
#define logd(format, args...)
#endif

#define ARRAY_SIZEOF(arr)	sizeof(arr)/sizeof(arr[0])

#define MIN(x,y) ((x)>(y) ? (y) : (x))

extern int doip_printf_hex(unsigned char *data, int len);

extern void doip_assert(uint8_t expr, const char *comment);

#endif
