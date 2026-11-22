#include <rtthread.h>

#include "boot_qspi.h"

#define APP_BASE 0x90000000UL

int main(void)
{
    uint32_t jedec_id;
    uint32_t app_sp;
    uint32_t app_entry;

    rt_kprintf("\n[Boot] RT-Thread QSPI bootloader\n");

    boot_qspi_init();
    jedec_id = boot_qspi_read_jedec_id();
    rt_kprintf("[Boot] QSPI JEDEC ID: %06X\n", jedec_id);

    if ((jedec_id == 0x000000UL) || (jedec_id == 0xFFFFFFUL))
    {
        rt_kprintf("[Boot] QSPI flash not detected, stop.\n");
        while (1)
        {
            rt_thread_mdelay(1000);
        }
    }

    if (boot_qspi_map_app() != 0)
    {
        rt_kprintf("[Boot] QSPI mapped in 4-bit mode.\n");
    }
    else
    {
        rt_kprintf("[Boot] QSPI mapped in 1-bit mode.\n");
    }

    if (boot_app_validate(APP_BASE, &app_sp, &app_entry) != 0)
    {
        rt_kprintf("[Boot] No valid application at 0x%08X.\n", APP_BASE);
        while (1)
        {
            rt_thread_mdelay(1000);
        }
    }

    rt_kprintf("[Boot] Application vector: SP=0x%08X entry=0x%08X\n",
               app_sp, app_entry);
    rt_kprintf("[Boot] Jumping to application at 0x%08X\n", APP_BASE);
    rt_thread_mdelay(20);

    boot_app_jump(APP_BASE, app_sp, app_entry);
}
