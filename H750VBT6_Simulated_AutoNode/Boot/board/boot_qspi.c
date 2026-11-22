#include <rtthread.h>
#include <stm32h7xx.h>

#include "boot_qspi.h"

#define QSPI_CCR_READ_ID        0x0500019FUL
#define QSPI_CCR_READ_SR1       0x05000105UL
#define QSPI_CCR_READ_SR2       0x05000135UL
#define QSPI_CCR_WRITE_ENABLE   0x00000106UL
#define QSPI_CCR_WRITE_SR2      0x01000031UL
#define QSPI_CCR_MMAP_SINGLE    0x0D20250BUL
#define QSPI_CCR_MMAP_QUAD      0x0F20256BUL
#define QSPI_CR_8MHZ            0x07000019UL
#define QSPI_DCR_16MB           0x00170400UL
#define QSPI_SR1_BUSY           0x01U
#define QSPI_SR2_QE             0x02U

#define APP_SP_MIN            0x24000000UL
#define APP_SP_MAX            0x24080000UL

static int qspi_wait_tcf(uint32_t timeout)
{
    while ((QUADSPI->SR & QUADSPI_SR_TCF) == 0U)
    {
        if (timeout-- == 0U)
        {
            return -1;
        }
    }

    return 0;
}

static int qspi_read_byte(uint32_t ccr, uint8_t *value)
{
    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    QUADSPI->DLR = 0;
    QUADSPI->CCR = ccr;

    if (qspi_wait_tcf(1000000UL) != 0)
    {
        return -1;
    }

    *value = *(volatile uint8_t *)&QUADSPI->DR;
    return 0;
}

static int qspi_write_enable(void)
{
    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    QUADSPI->CCR = QSPI_CCR_WRITE_ENABLE;

    return qspi_wait_tcf(1000000UL);
}

static int qspi_write_status2(uint8_t value)
{
    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    QUADSPI->DLR = 0;
    QUADSPI->CCR = QSPI_CCR_WRITE_SR2;
    *(volatile uint8_t *)&QUADSPI->DR = value;

    return qspi_wait_tcf(1000000UL);
}

static int qspi_wait_ready(uint32_t timeout)
{
    uint8_t status;

    while (timeout-- != 0U)
    {
        if (qspi_read_byte(QSPI_CCR_READ_SR1, &status) != 0)
        {
            return -1;
        }

        if ((status & QSPI_SR1_BUSY) == 0U)
        {
            return 0;
        }
    }

    return -1;
}

static int qspi_enable_quad_mode(void)
{
    uint8_t status;

    if (qspi_read_byte(QSPI_CCR_READ_SR2, &status) != 0)
    {
        return -1;
    }

    if ((status & QSPI_SR2_QE) != 0U)
    {
        return 0;
    }

    if (qspi_wait_ready(100000UL) != 0)
    {
        return -1;
    }

    if ((qspi_write_enable() != 0) ||
        (qspi_write_status2((uint8_t)(status | QSPI_SR2_QE)) != 0) ||
        (qspi_wait_ready(100000UL) != 0))
    {
        return -1;
    }

    if ((qspi_read_byte(QSPI_CCR_READ_SR2, &status) != 0) ||
        ((status & QSPI_SR2_QE) == 0U))
    {
        return -1;
    }

    return 0;
}

static void qspi_gpio_init(void)
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIODEN | RCC_AHB4ENR_GPIOEEN;
    __DSB();

    GPIOB->MODER = (GPIOB->MODER & ~((3UL << 4) | (3UL << 12))) |
                   (2UL << 4) | (2UL << 12);
    GPIOB->OSPEEDR |= (3UL << 4) | (3UL << 12);
    GPIOB->PUPDR = (GPIOB->PUPDR & ~((3UL << 4) | (3UL << 12))) | (1UL << 12);
    GPIOB->AFR[0] = (GPIOB->AFR[0] & ~((0xFUL << 8) | (0xFUL << 24))) |
                    (9UL << 8) | (10UL << 24);

    GPIOD->MODER = (GPIOD->MODER & ~(0x3FUL << 22)) | (0x2AUL << 22);
    GPIOD->OSPEEDR |= 0x3FUL << 22;
    GPIOD->PUPDR = (GPIOD->PUPDR & ~(0x3FUL << 22)) | (0x15UL << 22);
    GPIOD->AFR[1] = (GPIOD->AFR[1] & ~(0xFFFUL << 12)) | (0x999UL << 12);

    GPIOE->MODER = (GPIOE->MODER & ~(3UL << 4)) | (2UL << 4);
    GPIOE->OSPEEDR |= 3UL << 4;
    GPIOE->PUPDR = (GPIOE->PUPDR & ~(3UL << 4)) | (1UL << 4);
    GPIOE->AFR[0] = (GPIOE->AFR[0] & ~(0xFUL << 8)) | (9UL << 8);

    __DSB();
}

int boot_qspi_init(void)
{
    RCC->AHB3RSTR |= RCC_AHB3RSTR_QSPIRST;
    __DSB();
    RCC->AHB3RSTR &= ~RCC_AHB3RSTR_QSPIRST;
    RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN;
    __DSB();

    qspi_gpio_init();

    QUADSPI->CR = 0;
    QUADSPI->DCR = QSPI_DCR_16MB;
    QUADSPI->CR = QSPI_CR_8MHZ;

    for (volatile uint32_t delay = 0; delay < 100000UL; delay++)
    {
    }

    return 0;
}

uint32_t boot_qspi_read_jedec_id(void)
{
    uint32_t id = 0xFFFFFFFFUL;

    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    QUADSPI->DLR = 2;
    QUADSPI->CCR = QSPI_CCR_READ_ID;

    if (qspi_wait_tcf(1000000UL) != 0)
    {
        return id;
    }

    id  = (uint32_t)(*(volatile uint8_t *)&QUADSPI->DR) << 16;
    id |= (uint32_t)(*(volatile uint8_t *)&QUADSPI->DR) << 8;
    id |= (uint32_t)(*(volatile uint8_t *)&QUADSPI->DR);

    QUADSPI->FCR = QUADSPI_FCR_CTCF;

    return id;
}

int boot_qspi_map_app(void)
{
    int quad_mode = 0;

    if (qspi_enable_quad_mode() == 0)
    {
        quad_mode = 1;
    }

    QUADSPI->FCR = QUADSPI_FCR_CTCF;
    QUADSPI->AR = 0;
    QUADSPI->CCR = quad_mode ? QSPI_CCR_MMAP_QUAD : QSPI_CCR_MMAP_SINGLE;
    __DSB();
    __ISB();

    return quad_mode;
}

int boot_app_validate(uint32_t app_base, uint32_t *app_sp, uint32_t *app_entry)
{
    volatile const uint32_t *vector = (volatile const uint32_t *)app_base;
    uint32_t sp = vector[0];
    uint32_t entry = vector[1];
    uint32_t entry_address = entry & ~0x1UL;

    if ((sp < APP_SP_MIN) || (sp > APP_SP_MAX) || ((sp & 0x7UL) != 0U))
    {
        return -1;
    }

    if ((entry & 0x1UL) == 0U)
    {
        return -2;
    }

    if ((entry_address < app_base) ||
        (entry_address >= (app_base + 16UL * 1024UL * 1024UL)))
    {
        return -3;
    }

    *app_sp = sp;
    *app_entry = entry;
    return 0;
}

void boot_app_jump(uint32_t app_base, uint32_t app_sp, uint32_t app_entry)
{
    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    for (uint32_t index = 0; index < 8U; index++)
    {
        NVIC->ICER[index] = 0xFFFFFFFFUL;
        NVIC->ICPR[index] = 0xFFFFFFFFUL;
    }

    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;
    SCB->CCR &= ~(SCB_CCR_IC_Msk | SCB_CCR_DC_Msk);
    __DSB();
    __ISB();

    SCB->ICIALLU = 0UL;
    SCB_InvalidateDCache();
    SCB->VTOR = app_base;

    __DSB();
    __ISB();

    __asm volatile(
        "msr msp, %0\n"
        "bx  %1\n"
        :
        : "r"(app_sp), "r"(app_entry)
        : "memory");

    __builtin_unreachable();
}
