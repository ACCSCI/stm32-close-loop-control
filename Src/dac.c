/**
 * @file    dac.c
 * @brief   DAC Channel 1 output on PA4
 *
 * Pin:  PA4 (DAC_OUT1)
 * Range: 0 - 3.3V (12-bit, 0-4095)
 * Trigger: Software trigger
 */
#include "dac.h"
#include "system.h"

static uint16_t current_value = 0;

void dac_init(void)
{
    /* Enable clocks: GPIOA, DAC */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;

    /* PA4 -> analog mode (MODER = 11) */
    GPIOA->MODER |= (3U << 8);   /* PA4 = analog */

    /* DAC Channel 1:
     * EN1 = 1          -> enable channel 1
     * BOFF1 = 0        -> buffer enabled (better drive capability)
     * TEN1 = 0         -> no trigger needed for software mode
     * TSEL1 = 111      -> software trigger
     * WAVE1 = 00       -> disable wave generation
     * MAMP1 = 0000     -> mask (not used)
     */
    DAC->CR = DAC_CR_EN1 | DAC_CR_TSEL1_SW;

    current_value = 0;
}

void dac_set_value(uint16_t value)
{
    if (value > 4095) value = 4095;
    current_value = value;
    DAC->DHR12R1 = value;
    /* Software trigger: set SWTRIGR bit 0 */
    DAC->SWTRIGR = 1U;
}

void dac_set_voltage(float voltage)
{
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 3.3f) voltage = 3.3f;

    uint16_t value = (uint16_t)((voltage / 3.3f) * 4095.0f);
    dac_set_value(value);
}

uint16_t dac_get_value(void)
{
    return current_value;
}
