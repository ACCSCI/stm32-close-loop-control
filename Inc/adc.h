/**
 * @file    adc.h
 * @brief   ADC1 driver with analog watchdog
 */
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* Initialize ADC1 Channel 0 (PA0) for voltage measurement */
void adc_init(void);

/* Single conversion: read ADC value (0-4095) */
uint16_t adc_read(void);

/* Read voltage (0.0 - 3.3V) */
float adc_read_voltage(void);

/* Read the STM32 internal temperature sensor (ADC1_IN16).
 * Returns the die temperature in degrees Celsius.
 * Temporarily switches ADC1 to channel 16, then restores channel 0. */
float adc_read_temperature(void);

/* Read internal VREFINT (ADC1_IN17, ~1.21V bandgap). Returns 12-bit raw. */
uint16_t adc_read_vrefint_raw(void);

/* Configure analog watchdog thresholds (12-bit values) */
void adc_set_watchdog_thresholds(uint16_t high, uint16_t low);

/* Enable/disable analog watchdog */
void adc_watchdog_enable(uint8_t enable);

/* Check if watchdog alarm is active */
uint8_t adc_watchdog_alarm(void);

/* Clear watchdog alarm flag */
void adc_watchdog_clear(void);

#endif /* ADC_H */
