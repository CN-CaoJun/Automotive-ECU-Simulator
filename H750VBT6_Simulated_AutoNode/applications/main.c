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

int main(void)
{
    int count = 0;

    int x = 0x200000;

    rt_kprintf("%x\r\n",(x >> 16));

    rt_kprintf("%x\r\n",((x >> 16) && 0x20));
    return RT_EOK;
}
