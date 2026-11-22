#ifndef BOOT_QSPI_H
#define BOOT_QSPI_H

#include <stdint.h>

int boot_qspi_init(void);
uint32_t boot_qspi_read_jedec_id(void);
int boot_qspi_map_app(void);
int boot_app_validate(uint32_t app_base, uint32_t *app_sp, uint32_t *app_entry);
void boot_app_jump(uint32_t app_base, uint32_t app_sp, uint32_t app_entry) __attribute__((noreturn));

#endif
