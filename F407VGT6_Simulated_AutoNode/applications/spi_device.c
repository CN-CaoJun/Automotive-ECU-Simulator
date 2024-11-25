/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-09     dj461v       the first version
 */
/*****************************************************************************************/
#include <rtthread.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

#define ATTACH_W25Q64 1
static int rt_hw_spi_flash_init(void)
{
    #if ATTACH_W25Q64
    rt_hw_spi_device_attach("spi2", "spi20", GPIOE, GPIO_PIN_3);
    if (RT_NULL == rt_sfud_flash_probe("W25Q64", "spi20"))
    {
        return -RT_ERROR;
    };
    #else
        
    #endif
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);


