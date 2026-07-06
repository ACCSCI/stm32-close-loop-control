/**
 * @file    system.h
 * @brief   STM32F407 register definitions for closed-loop control project
 */
#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

/*---------- Compiler hints-----------*/
#define __IO    volatile
#define __I     volatile const
#define __O     volatile

/*---------- NVIC Registers-----------*/
#define NVIC_ISER0      (*(volatile uint32_t*)0xE000E100)
#define NVIC_ISER1      (*(volatile uint32_t*)0xE000E104)

/*---------- SysTick Registers-----------*/
#define SysTick         ((SysTick_Type*)0xE000E010)
typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __I  uint32_t CALIB;
} SysTick_Type;

#define SysTick_CTRL_CLKSOURCE_Msk  (1U << 2)
#define SysTick_CTRL_TICKINT_Msk    (1U << 1)
#define SysTick_CTRL_ENABLE_Msk     (1U << 0)
#define SysTick_CTRL_COUNTFLAG_Msk  (1U << 16)

/*---------- RCC Registers-----------*/
#define RCC             ((RCC_Type*)0x40023800)
typedef struct {
    __IO uint32_t CR;
    __IO uint32_t PLLCFGR;
    __IO uint32_t CFGR;
    __IO uint32_t CIR;
    __IO uint32_t AHB1RSTR;
    __IO uint32_t AHB2RSTR;
    __IO uint32_t AHB3RSTR;
    uint32_t      RESERVED0;
    __IO uint32_t APB1RSTR;
    __IO uint32_t APB2RSTR;
    uint32_t      RESERVED1[2];
    __IO uint32_t AHB1ENR;
    __IO uint32_t AHB2ENR;
    __IO uint32_t AHB3ENR;
    uint32_t      RESERVED2;
    __IO uint32_t APB1ENR;
    __IO uint32_t APB2ENR;
    uint32_t      RESERVED3[2];
    __IO uint32_t AHB1LPENR;
    __IO uint32_t AHB2LPENR;
    __IO uint32_t AHB3LPENR;
    uint32_t      RESERVED4;
    __IO uint32_t APB1LPENR;
    __IO uint32_t APB2LPENR;
    uint32_t      RESERVED5[2];
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
    uint32_t      RESERVED6[2];
    __IO uint32_t SSCGR;
    __IO uint32_t PLLI2SCFGR;
} RCC_Type;

/* RCC_CR bits */
#define RCC_CR_HSION        (1U << 0)
#define RCC_CR_HSIRDY       (1U << 1)
#define RCC_CR_HSEON        (1U << 16)
#define RCC_CR_HSERDY       (1U << 17)
#define RCC_CR_PLLON        (1U << 24)
#define RCC_CR_PLLRDY       (1U << 25)

/* RCC_PLLCFGR bits */
#define RCC_PLLCFGR_PLLSRC_HSE (1U << 22)

/* RCC_CFGR bits */
#define RCC_CFGR_SW_HSI     (0U << 0)
#define RCC_CFGR_SW_HSE     (1U << 0)
#define RCC_CFGR_SW_PLL     (2U << 0)
#define RCC_CFGR_SWS_HSI    (0U << 2)
#define RCC_CFGR_SWS_HSE    (1U << 2)
#define RCC_CFGR_SWS_PLL    (2U << 2)
#define RCC_CFGR_PPRE1_DIV2 (4U << 10)
#define RCC_CFGR_PPRE2_DIV1 (0U << 13)
#define RCC_CFGR_HPRE_DIV1  (0U << 4)

/* RCC_AHB1ENR bits */
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
#define RCC_AHB1ENR_GPIOBEN (1U << 1)
#define RCC_AHB1ENR_GPIOCEN (1U << 2)
#define RCC_AHB1ENR_GPIODEN (1U << 3)
#define RCC_AHB1ENR_GPIOEEN (1U << 4)
#define RCC_AHB1ENR_GPIOFEN (1U << 5)
#define RCC_AHB1ENR_GPIOGEN (1U << 6)
#define RCC_AHB1ENR_DMA1EN  (1U << 21)

/* RCC_AHB3ENR bits */
#define RCC_AHB3ENR_FSMCEN  (1U << 0)

/* RCC_APB1ENR bits */
#define RCC_APB1ENR_USART2EN (1U << 17)
#define RCC_APB1ENR_DACEN    (1U << 29)
#define RCC_APB1ENR_PWREN    (1U << 28)

/* RCC_APB2ENR bits */
#define RCC_APB2ENR_USART1EN (1U << 4)
#define RCC_APB2ENR_ADC1EN   (1U << 8)
#define RCC_APB2ENR_ADC2EN   (1U << 9)
#define RCC_APB2ENR_ADC3EN   (1U << 10)

/*---------- GPIO Registers-----------*/
#define GPIOA           ((GPIO_Type*)0x40020000)
#define GPIOB           ((GPIO_Type*)0x40020400)
#define GPIOC           ((GPIO_Type*)0x40020800)
#define GPIOD           ((GPIO_Type*)0x40020C00)
#define GPIOE           ((GPIO_Type*)0x40021000)
#define GPIOF           ((GPIO_Type*)0x40021400)
#define GPIOG           ((GPIO_Type*)0x40021800)
typedef struct {
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __I  uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_Type;

/* GPIO mode: 00=input, 01=output, 10=AF, 11=analog */
/*---------- DAC Registers-----------*/
#define DAC             ((DAC_Type*)0x40007400)
typedef struct {
    __IO uint32_t CR;
    __IO uint32_t SWTRIGR;
    __IO uint32_t DHR12R1;
    __IO uint32_t DHR12L1;
    __IO uint32_t DHR8R1;
    __IO uint32_t DHR12R2;
    __IO uint32_t DHR12L2;
    __IO uint32_t DHR8R2;
    __IO uint32_t DHR12RD;
    __IO uint32_t DHR12LD;
    __IO uint32_t DHR8RD;
    __IO uint32_t DOR1;
    __IO uint32_t DOR2;
    __IO uint32_t SR;
} DAC_Type;

/* DAC_CR bits */
#define DAC_CR_EN1      (1U << 0)
#define DAC_CR_BOFF1    (1U << 1)
#define DAC_CR_TEN1     (1U << 2)
#define DAC_CR_TSEL1_SW (7U << 3)   /* software trigger */
#define DAC_CR_WAVE1_DISABLED (0U << 6)
#define DAC_CR_MAMP1_4095 (0U << 8)
#define DAC_CR_EN2      (1U << 16)
#define DAC_CR_BOFF2    (1U << 17)
#define DAC_CR_TEN2     (1U << 18)
#define DAC_CR_TSEL2_SW (7U << 19)
#define DAC_CR_WAVE2_DISABLED (0U << 22)

/*---------- ADC Registers-----------*/
#define ADC1            ((ADC_Type*)0x40012000)
#define ADC2            ((ADC_Type*)0x40012100)
#define ADC3            ((ADC_Type*)0x40012200)
typedef struct {
    __IO uint32_t SR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMPR1;
    __IO uint32_t SMPR2;
    __IO uint32_t JOFR1;
    __IO uint32_t JOFR2;
    __IO uint32_t JOFR3;
    __IO uint32_t JOFR4;
    __IO uint32_t HTR;
    __IO uint32_t LTR;
    __IO uint32_t SQR1;
    __IO uint32_t SQR2;
    __IO uint32_t SQR3;
    __IO uint32_t JSQR;
    __IO uint32_t JDR1;
    __IO uint32_t JDR2;
    __IO uint32_t JDR3;
    __IO uint32_t JDR4;
    __IO uint32_t DR;
} ADC_Type;

/* ADC_SR bits */
#define ADC_SR_EOC      (1U << 1)
#define ADC_SR_STRT     (1U << 4)
#define ADC_SR_AWD      (1U << 0)

/* ADC_CR1 bits */
#define ADC_CR1_EOCIE   (1U << 5)
#define ADC_CR1_AWDEN   (1U << 22)
#define ADC_CR1_AWDSGL  (1U << 21)
#define ADC_CR1_DISCEN  (1U << 11)

/* ADC_CR2 bits */
#define ADC_CR2_ADON    (1U << 0)
#define ADC_CR2_CONT    (1U << 1)
#define ADC_CR2_DMA     (1U << 8)
#define ADC_CR2_SWSTART (1U << 30)
#define ADC_CR2_EXTEN_RISING (1U << 28)
#define ADC_CR2_EXTSEL_SW    (7U << 24)

/* ADC sample time: 84 cycles for good accuracy */
#define ADC_SMPR_84     7U

/*---------- USART Registers-----------*/
#define USART1          ((USART_Type*)0x40011000)
#define USART2          ((USART_Type*)0x40004400)
typedef struct {
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t BRR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t CR3;
    __IO uint32_t GTPR;
} USART_Type;

/* USART_SR bits */
#define USART_SR_TXE    (1U << 7)
#define USART_SR_TC     (1U << 6)
#define USART_SR_RXNE   (1U << 5)
#define USART_SR_ORE    (1U << 3)

/* USART_CR1 bits */
#define USART_CR1_UE    (1U << 13)
#define USART_CR1_TE    (1U << 3)
#define USART_CR1_RE    (1U << 2)
#define USART_CR1_RXNEIE (1U << 5)

/*---------- EXTI Registers-----------*/
#define EXTI            ((EXTI_Type*)0x40013C00)
typedef struct {
    __IO uint32_t IMR;
    __IO uint32_t EMR;
    __IO uint32_t RTSR;
    __IO uint32_t FTSR;
    __IO uint32_t SWIER;
    __IO uint32_t PR;
} EXTI_Type;

/*---------- SYSCFG Registers-----------*/
#define SYSCFG          ((SYSCFG_Type*)0x40013800)
typedef struct {
    uint32_t      MEMRMP;
    uint32_t      PMC;
    __IO uint32_t EXTICR[4];
    uint32_t      RESERVED[2];
    __IO uint32_t CMPCR;
} SYSCFG_Type;

/*---------- FSMC Registers (for TFT LCD via NOR/SRAM Bank1) -----------*/
/* Control registers block: BCR/BTR interleaved at 0xA0000000.
 * BTCR[0]=BCR1, BTCR[1]=BTR1, ... BTCR[6]=BCR4, BTCR[7]=BTR4 */
#define FSMC_Bank1      ((FSMC_Bank1_Type*)0xA0000000)
typedef struct {
    __IO uint32_t BTCR[8];
} FSMC_Bank1_Type;

/* Extended write-timing registers block at 0xA0000104.
 * BWTR[0]=BWTR1, BWTR[1]=BWTR2, BWTR[2]=BWTR3, BWTR[3]=BWTR4.
 * NOTE: BWTR[4..6] do NOT exist in the register map — the array
 * accesses that index before the fix hit 0xA000011C instead of 0xA0000110. */
#define FSMC_Bank1E     ((FSMC_Bank1E_Type*)0xA0000104)
typedef struct {
    __IO uint32_t BWTR[4];
} FSMC_Bank1E_Type;

/* FSMC_BCRx bits */
#define FSMC_BCR_MBKEN   (1U << 0)    /* memory bank enable */
#define FSMC_BCR_MTYP_SRAM (0U << 2)  /* memory type = SRAM */
#define FSMC_BCR_MWID_16 (1U << 4)    /* memory data bus width = 16-bit */
#define FSMC_BCR_WREN    (1U << 12)   /* write enable */
#define FSMC_BCR_EXTMOD  (1U << 14)   /* extended mode (separate read/write timing) */

/*---------- Function prototypes-----------*/
void system_init(void);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
uint32_t get_tick(void);

/* Clock frequency */
#define SYSCLK_FREQ     168000000UL
#define APB1_FREQ       84000000UL
#define APB2_FREQ       168000000UL

#endif /* SYSTEM_H */
