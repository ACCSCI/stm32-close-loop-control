/**
 * @file    adc.c
 * @brief   Two-ADCs architecture for clean closed-loop + temp sensing.
 *
 *   ADC2 -> PA0 (ADC1_IN0): closed-loop feedback + analog watchdog.
 *            Used by the main loop at 50ms cadence; never touches other channels,
 *            so its DR register is never stale.
 *   ADC1 -> Internal temp sensor (CH16) + VREFINT (CH17).
 *            Used by adc_read_temperature() every 500ms. Independent of ADC2.
 *
 * Pin:    PA0 (ADC2_IN0)
 * Trigger: Software on each ADC
 * Mode:   Single conversion, polled
 */
#include "adc.h"
#include "system.h"

static volatile uint8_t awd_alarm_flag = 0;

/*---------- ADC2 IRQ Handler (analog watchdog on PA0) ----------*/
void ADC_IRQHandler(void)
{
    /* ADC1 and ADC2 share IRQ#18. Only the analog watchdog of ADC2 is enabled
     * here, so we can just clear the flag on ADC2. */
    if (ADC2->SR & ADC_SR_AWD) {
        awd_alarm_flag = 1;
        ADC2->SR &= ~ADC_SR_AWD;
    }
}

void adc_init(void)
{
    /* Clocks: GPIOA, ADC1, ADC2 */
    RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR  |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_ADC2EN;

    /* PA5 -> analog mode (silk-screen "ADC/TPAD" on ALIENTEK Explorer F407).
     * NOTE: the silk-screen label "PA0" is the WKUP-only pin on this board;
     * the silk-screen label "ADC" (or "TPAD") is actually PA5 (ADC2_IN5),
     * which is what the closed-loop feedback wire goes to. */
    GPIOA->MODER |= (3U << (5 * 2));    /* PA5 = analog */

    /* ADC common (CCR @ 0x40012304):
     *   ADCPRE = 01  -> /4 -> ADCCLK = 84/4 = 21 MHz
     *   TSVREFE = 1  -> enable internal temperature sensor & Vrefint on ADC1 */
    *((volatile uint32_t*)0x40012304) = (1U << 16) | (1U << 23);

    /* ====== ADC2: PA0 closed-loop feedback ======
     * CR1: AWDSGL=1, AWDCH=0 -> single-channel watchdog on channel 0 (PA0).
     * The temperature sensor (CH16) and VREFINT (CH17) live on ADC1 only,
     * so they cannot spuriously trip ADC2's watchdog. */
    ADC2->CR1 = ADC_CR1_AWDSGL;
    ADC2->CR2 = ADC_CR2_ADON;
    ADC2->SMPR2 = (7U << (5 * 3)); /* CH5 (PA5) = 84 cycles (SMPR2 bits [17:15]) */
    ADC2->SQR1 = 0;                /* L[3:0] = 0 -> 1 conversion */
    ADC2->SQR3 = 5;                /* SQ1 = channel 5 (PA5) */
    /* Stabilization delay (datasheet: tSTAB ~ 1 us after ADON). */
    for (volatile int i = 0; i < 10000; i++);
    /* Second ADON set: the first SWSTART after power-up is ignored on some
     * STM32F4 revisions. Writing CR2 with ADON again (keep it 1) wakes the
     * ADC properly so that the next SWSTART triggers EOC. Do NOT clear ADON
     * in between — that resets the ADC and loses the register config. */
    ADC2->CR2 = ADC_CR2_ADON;
    for (volatile int i = 0; i < 10000; i++);

    /* ====== ADC1: temperature sensor + VREFINT ======
     * ADC1 will not be used for PA0, so no SQR3/scan setup is needed beyond
     * power-up. We set CR1 to default (no watchdog on ADC1). */
    ADC1->CR1 = 0;
    ADC1->CR2 = ADC_CR2_ADON;
    /* CH16 (temp sensor) needs 480-cycle sample time. */
    ADC1->SMPR1 = (7U << 18);      /* SMPR1[20:18] = 7 -> CH16 = 480 cycles */
    for (volatile int i = 0; i < 10000; i++);
    /* Second ADON set (keep ADON=1, do NOT toggle 0->1 — see ADC2 above). */
    ADC1->CR2 = ADC_CR2_ADON;
    for (volatile int i = 0; i < 10000; i++);

    /* NVIC IRQ 18 is shared by ADC1/ADC2/ADC3. We use it for the ADC2 watchdog. */
    NVIC_ISER1 |= (1U << 18);
}

/* Read PA0 (ADC2 channel 0). Single conversion. Reading DR clears EOC. */
uint16_t adc_read(void)
{
    ADC2->SR &= ~ADC_SR_EOC;
    ADC2->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC2->SR & ADC_SR_EOC));
    return (uint16_t)(ADC2->DR & 0xFFF);
}

float adc_read_voltage(void)
{
    uint16_t raw = adc_read();
    return (float)raw * 3.3f / 4095.0f;
}

/* Read the internal temperature sensor (ADC1_IN16). VREFINT (ADC1_IN17)
 * is read in the same call to compute Vdda so the result is independent of
 * the actual supply voltage. */
float adc_read_temperature(void)
{
    uint32_t acc = 0;
    const int samples = 8;

    ADC1->SQR3 = 16;                 /* SQ1 = channel 16 (temp sensor) */
    for (int i = 0; i < samples; i++) {
        ADC1->SR &= ~ADC_SR_EOC;
        ADC1->CR2 |= ADC_CR2_SWSTART;
        while (!(ADC1->SR & ADC_SR_EOC));
        acc += (ADC1->DR & 0xFFF);
    }

    /* VREFINT (1.21V bandgap reference, ADC1_IN17) for Vdda calibration. */
    ADC1->SQR3 = 17;
    ADC1->SR &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    uint16_t vref_raw = (uint16_t)(ADC1->DR & 0xFFF);

    float vdda = 1.21f * 4095.0f / (float)vref_raw;
    float raw_avg = (float)acc / (float)samples;
    float v_sense = raw_avg * vdda / 4095.0f;
    return (v_sense - 0.76f) / 0.0025f + 25.0f;
}

/* Read VREFINT (ADC1_IN17, ~1.21V bandgap). Returns 12-bit raw. */
uint16_t adc_read_vrefint_raw(void)
{
    ADC1->SQR3 = 17;
    ADC1->SR &= ~ADC_SR_EOC;
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC));
    return (uint16_t)(ADC1->DR & 0xFFF);
}

void adc_set_watchdog_thresholds(uint16_t high, uint16_t low)
{
    if (high > 4095) high = 4095;
    if (low > 4095) low = 4095;
    ADC2->HTR = high;
    ADC2->LTR = low;
}

void adc_watchdog_enable(uint8_t enable)
{
    if (enable) ADC2->CR1 |= ADC_CR1_AWDEN;
    else        ADC2->CR1 &= ~ADC_CR1_AWDEN;
    awd_alarm_flag = 0;
}

uint8_t adc_watchdog_alarm(void)
{
    return awd_alarm_flag;
}

void adc_watchdog_clear(void)
{
    awd_alarm_flag = 0;
}