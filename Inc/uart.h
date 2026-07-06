/**
 * @file    uart.h
 * @brief   USART1 (PA9/PA10) driver for serial communication
 */
#ifndef UART_H
#define UART_H

#include <stdint.h>

/* Initialize USART1 at given baud rate */
void uart_init(uint32_t baudrate);

/* Blocking send */
void uart_send_char(char c);
void uart_send_string(const char *str);
void uart_send_number(uint32_t num);

/* Receive (non-blocking, returns -1 if nothing available) */
int uart_receive_char(void);

/* printf-like function (minimal) */
void uart_printf(const char *fmt, ...);

#endif /* UART_H */
