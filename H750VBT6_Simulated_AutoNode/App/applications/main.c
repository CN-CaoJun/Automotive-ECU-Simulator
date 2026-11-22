/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-22     Cao Jun    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

extern int rtcan_node_start(void);
static void timer_callback(void *parameter)
{
    static int count = 0;
    count++;
    // rt_kprintf("Timer test %d \n", count);
}

int main(void)
{
    rtcan_node_start();
    rt_timer_t timer = rt_timer_create("timer1",  
                                       timer_callback,  
                                       RT_NULL,  
                                       100,  
                                       RT_TIMER_FLAG_PERIODIC);  

    if (timer != RT_NULL)
    {
        rt_timer_start(timer);
    }
    else
    {
        rt_kprintf("Failed to create timer\n");
    }

    while (1)
    {
        rt_thread_mdelay(1000); 
    }

    return RT_EOK;
}
