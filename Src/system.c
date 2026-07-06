/**
 * @file    system.c
 * @brief   System clock init (168MHz), SysTick delay, GPIO/clock enable
 */
#include "system.h"

static volatile uint32_t s_ticks_ms = 0;

uint32_t get_tick(void) { return s_ticks_ms; }

/*---------- SysTick IRQ handler ----------*/
void SysTick_Handler(void)
{
    s_ticks_ms++;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = s_ticks_ms;
    while ((s_ticks_ms - start) < ms);
}

void delay_us(uint32_t us)
{
    /* Busy-wait using DWT-free reload trick on a temporarily reprogrammed
     * SysTick. SysTick counts at 168MHz, reload = 168 -> 1us. */
    SysTick->CTRL = 0;                 /* stop, so the reload below is clean */
    SysTick->LOAD = 168 * us - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));

    /* Restore the periodic 1ms system tick (interrupt driven) so that
     * get_tick()/delay_ms() keep working after a us delay. */
    SysTick->CTRL = 0;
    SysTick->LOAD = 168000 - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

/*---------- System clock: 168 MHz from 16 MHz HSI ----------*/
void system_init(void)
{
    /* Enable the FPU (CP10 & CP11 full access). The project is built with
     * -mfloat-abi=hard, so any float op before this faults with UFSR.NOCP.
     * CPACR is at 0xE000ED88; set bits [23:20] = 0xF. */
    *((volatile uint32_t*)0xE000ED88) |= (0xFU << 20);
    __asm volatile ("dsb");
    __asm volatile ("isb");

    /* Enable HSI */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    /* Configure PLL: HSI=16MHz -> VCO=336MHz, SYSCLK=168MHz
     * PLLM=16, PLLN=336, PLLP=2, PLLSRC=HSI
     * VCO = HSI/PLLM * PLLN = 16/16*336 = 336 MHz
     * SYSCLK = VCO/PLLP = 336/2 = 168 MHz
     */
    RCC->PLLCFGR = (16U  << 0)   |   /* PLLM = 16  */
                    (336U << 6)  |   /* PLLN = 336  */
                    (0U  << 16)  |   /* PLLP = /2   */
                    (0U  << 22)  |   /* PLLSRC = HSI (default) */
                    (7U  << 24);     /* PLLQ = 7    */

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* AHB=168MHz, APB1=42MHz, APB2=84MHz */
    RCC->CFGR = RCC_CFGR_HPRE_DIV1 |
                RCC_CFGR_PPRE1_DIV2 |
                RCC_CFGR_PPRE2_DIV1;

    /* Flash: 5 wait states for 168MHz, prefetch, instruction cache, data cache.
     * FLASH_ACR = LATENCY(5) | PRFTEN(bit8) | ICEN(bit9) | DCEN(bit10) = 0x705.
     * NOTE: the value MUST live in the low bits; 0x705D0000 would leave
     * LATENCY=0 and bus-fault when the CPU fetches from flash at 168MHz. */
    *((volatile uint32_t*)0x40023C00) = 0x00000705;

    /* Switch system clock to PLL */
    RCC->CFGR = (RCC->CFGR & ~0x3U) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & 0xCU) != RCC_CFGR_SWS_PLL);

    /* SysTick: 168MHz / 1000 = 168000 ticks per ms */
    SysTick->LOAD = 168000 - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;

    /* Enable SysTick IRQ (NVIC IRQ 15) */
    NVIC_ISER0 |= (1U << 15);
}
