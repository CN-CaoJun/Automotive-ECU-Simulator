/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-08     RT-Thread    first version
 */

#include <rtthread.h>
#include <sys/select.h>

#ifndef ULOG_USING_SYSLOG
#define LOG_TAG              "Tick"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#else
#include <syslog.h>
#endif /* ULOG_USING_SYSLOG */

#define THREAD_STACK_SIZE   512 // 定义任务堆栈大小
#define THREAD_PRIORITY     20  // 定义任务优先级

#define THREAD_STACK_SIZE   512
#define THREAD_PRIORITY     20

static int count = 0;
static void thread_entry(void *parameter) {
    while (1) {
        count++;
        LOG_D("Tick - %d",count);
        if (count == 1)
        {
           // ulog_example();
        }
        
        rt_thread_mdelay(1000);
    }
}

int main(void) {

    return 0;
}


void tick_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("local", thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, 20);

    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    }
}
MSH_CMD_EXPORT(tick_init, run ulog example)


