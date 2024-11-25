#ifndef __DOIP_TIMER_H__
#define __DOIP_TIMER_H__

#include <signal.h>
#include <sys/types.h>

/*------------------------------------------------------------------------------------------------------------*/

typedef void (*timer_callback)(union sigval val);

extern int timer_init(timer_t *timerid, timer_callback cb, void *data);

extern int timer_start(timer_t timerid, double after, double repeat);

extern int timer_stop(timer_t timerid);

#endif  /* __DOIP_TIMER_H__ */