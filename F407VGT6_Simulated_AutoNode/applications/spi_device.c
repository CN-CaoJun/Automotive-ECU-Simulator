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
#include <rtdevice.h>
#include <drv_spi.h>
#include <spi_flash.h>
#include <spi_flash_sfud.h>
#include <fal.h>
#include <dfs_fs.h>

#define DRV_DEBUG
#define LOG_TAG                         "drv.spiflash"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define FS_PARTITION_NAME               "filesystem"

#define SPI_CS_GPIO                     GPIOE
#define SPI_CS_PIN                      GPIO_PIN_3

static int rt_hw_spi_flash_with_sfud_init(void)
{
    rt_err_t err = RT_EOK;
    rt_hw_spi_device_attach("spi2", "spi20", SPI_CS_GPIO, SPI_CS_PIN);

    /* init W25Q64, And register as a block device */
    if (RT_NULL == rt_sfud_flash_probe(FAL_USING_NOR_FLASH_DEV_NAME, "spi20"))
    {
        LOG_E("Failed to probe flash device "FAL_USING_NOR_FLASH_DEV_NAME);
        return -RT_ERROR;
    }
    return err;
}
INIT_DEVICE_EXPORT(rt_hw_spi_flash_with_sfud_init);

static int mnt(void)
{
    struct rt_device *mtd_dev = RT_NULL;

    fal_init();
    mtd_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
    if (!mtd_dev)
    {
        LOG_E("Can't create a mtd device on '%s' partition.", FS_PARTITION_NAME);
    }
    else
    {
        // /* mount littlefs */
        // if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
        // {
        //     LOG_I("Filesystem initialized!");
        // }
        // else
        // {
        //     dfs_mkfs("lfs", FS_PARTITION_NAME);
        //     /* mount littlefs */
        //     if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
        //     {
        //         mkdir("/qspi",0x777);
        //         LOG_I("mkdir /qspi. Filesystem initialized!");
        //     }
        //     else
        //     {
        //         LOG_E("Failed to initialize filesystem!");
        //     }
        // }
    }
    return RT_EOK;
}
INIT_ENV_EXPORT(mnt);
