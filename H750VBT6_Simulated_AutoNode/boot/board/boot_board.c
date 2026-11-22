#include <rtthread.h>
#include <rthw.h>
#include <stm32h7xx.h>

extern uint32_t SystemCoreClock;

static void boot_uart_init(void)
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    __DSB();

    GPIOA->MODER = (GPIOA->MODER & ~((3UL << 18) | (3UL << 20))) |
                   (2UL << 18) | (2UL << 20);
    GPIOA->OSPEEDR |= (3UL << 18) | (3UL << 20);
    GPIOA->PUPDR = (GPIOA->PUPDR & ~((3UL << 18) | (3UL << 20))) |
                   (1UL << 18) | (1UL << 20);
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~((0xFUL << 4) | (0xFUL << 8))) |
                    (7UL << 4) | (7UL << 8);

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->BRR = 0x22C;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void boot_systick_init(void)
{
    SysTick->LOAD = (SystemCoreClock / RT_TICK_PER_SECOND) - 1UL;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, 0xFF);
}

void rt_hw_console_output(const char *str)
{
    while (*str != '\0')
    {
        if (*str == '\n')
        {
            while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0U)
            {
            }
            USART1->TDR = '\r';
        }

        while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0U)
        {
        }
        USART1->TDR = (uint8_t)*str++;
    }
}

void rt_hw_board_init(void)
{
    boot_uart_init();
    boot_systick_init();
}

void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}

void rt_hw_us_delay(rt_uint32_t us)
{
    volatile uint32_t count = us * (SystemCoreClock / 1000000UL);
    while (count-- != 0U)
    {
    }
}
