#include "doip_timer.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

int timer_init(timer_t *timerid, timer_callback cb, void *data)
{
    int ret;
    struct sigevent evp;

    // if (NULL == timerid) {
    //     return -1;
    // }
    memset(&evp, 0x00, sizeof(evp));
    evp.sigev_value.sival_ptr = data;
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_notify_function = cb;
    ret = timer_create(CLOCK_REALTIME, &evp, timerid);

    return ret;
}

int timer_start(timer_t timerid, double after, double repeat)
{
    int ret;
    struct itimerspec it;

    it.it_interval.tv_sec = (int)repeat;
    it.it_interval.tv_nsec = (int)((repeat-it.it_interval.tv_sec) * 1000000000);
    it.it_value.tv_sec = (int)after;
    it.it_value.tv_nsec = (int)((after-it.it_interval.tv_sec) * 1000000000)+1;
    ret = timer_settime(timerid, TIMER_ABSTIME, &it, NULL);

    return ret;
}

int timer_stop(timer_t timerid)
{
    return timer_delete(timerid);
}