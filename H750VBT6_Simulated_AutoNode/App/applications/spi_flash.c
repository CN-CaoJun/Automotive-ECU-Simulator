#include <rtthread.h>
#include <rtdevice.h>
#include <drv_spi.h>

#define W25Q64_BUS_NAME     "spi1"
#define W25Q64_DEVICE_NAME  "w25q64"

static rt_uint8_t w25q64_jedec_id[3];
static rt_bool_t w25q64_ready;

static int w25q64_probe(void)
{
    struct rt_spi_device *spi_device;
    struct rt_spi_configuration config =
    {
        .mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB,
        .data_width = 8,
        .max_hz = 10 * 1000 * 1000,
    };
    rt_uint8_t command = 0x9F;
    rt_err_t result;

    __HAL_RCC_GPIOD_CLK_ENABLE();

    result = rt_hw_spi_device_attach(W25Q64_BUS_NAME, W25Q64_DEVICE_NAME,
                                     GPIOD, GPIO_PIN_3);
    if (result != RT_EOK)
    {
        rt_kprintf("w25q64: attach failed (%d)\n", result);
        return result;
    }

    spi_device = (struct rt_spi_device *)rt_device_find(W25Q64_DEVICE_NAME);
    if (spi_device == RT_NULL)
    {
        rt_kprintf("w25q64: device not found\n");
        return -RT_ERROR;
    }

    result = rt_spi_configure(spi_device, &config);
    if (result != RT_EOK)
    {
        rt_kprintf("w25q64: configure failed (%d)\n", result);
        return result;
    }

    result = rt_spi_send_then_recv(spi_device, &command, sizeof(command),
                                   w25q64_jedec_id, sizeof(w25q64_jedec_id));
    if (result != RT_EOK)
    {
        rt_kprintf("w25q64: JEDEC read failed (%d)\n", result);
        return result;
    }

    w25q64_ready = w25q64_jedec_id[0] == 0xEF;
    rt_kprintf("w25q64: JEDEC ID %02X %02X %02X\n",
               w25q64_jedec_id[0], w25q64_jedec_id[1], w25q64_jedec_id[2]);

    return w25q64_ready ? RT_EOK : -RT_ERROR;
}

static int w25q64_info(void)
{
    if (!w25q64_ready)
    {
        rt_kprintf("w25q64: not detected\n");
        return -RT_ERROR;
    }

    rt_kprintf("w25q64: JEDEC ID %02X %02X %02X\n",
               w25q64_jedec_id[0], w25q64_jedec_id[1], w25q64_jedec_id[2]);
    return RT_EOK;
}

INIT_DEVICE_EXPORT(w25q64_probe);
MSH_CMD_EXPORT(w25q64_info, show SPI flash JEDEC ID);
