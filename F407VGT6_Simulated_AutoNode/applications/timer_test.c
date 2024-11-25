#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <time.h> 

#define DBG_TAG "TIMER_TEST"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

void timer_callback(union sigval sv) {
    printf("Timer expired.\n");
}

int timer_test() {
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    long long delay_nanoseconds = 1000000000LL; // 1 second

    // 设置回调函数
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_notify_function = timer_callback;
    sev.sigev_value.sival_ptr = &timerid;

    // 创建定时器
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("timer_create");
        // exit(EXIT_FAILURE);
    }

    // 设置定时器的间隔
    its.it_value.tv_sec = delay_nanoseconds / 1000000000;
    its.it_value.tv_nsec = delay_nanoseconds % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    // 启动定时器
    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        // c(EXIT_FAILURE);
    }

    // 等待一段时间以触发定时器
    // sleep(5);

    return 0;
}