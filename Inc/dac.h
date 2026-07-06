/**
 * @file    dac.h
 * @brief   DAC Channel 1 (PA4) driver
 */
#ifndef DAC_H
#define DAC_H

#include <stdint.h>

/* Initialize DAC Channel 1 on PA4 */
void dac_init(void);

/* Set DAC output value (0-4095 for 12-bit) */
void dac_set_value(uint16_t value);

/* Set DAC output voltage (0.0 - 3.3V) */
void dac_set_voltage(float voltage);

/* Get current DAC digital value */
uint16_t dac_get_value(void);

#endif /* DAC_H */
