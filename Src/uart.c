/**
 * @file    uart.c
 * @brief   USART1 driver - PA9(TX), PA10(RX), 115200 baud
 */
#include "uart.h"
#include "system.h"
#include <stdarg.h>
#include <stdio.h>

void uart_init(uint32_t baudrate)
{
    /* Enable clocks: GPIOA, USART1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9  -> AF7 (USART1_TX): MODER=AF(10), OSPEEDR=very-high(11), AF7=0111
     * PA10 -> AF7 (USART1_RX): MODER=AF(10), PUPDR=pull-up(01)
     */
    GPIOA->MODER    &= ~(0xFU << 18);          /* clear PA9, PA10 mode */
    GPIOA->MODER    |=  (0xAU << 18);          /* PA9=AF, PA10=AF */
    GPIOA->OSPEEDR  |=  (0xFU << 18);          /* very high speed */
    GPIOA->PUPDR    &= ~(0xFU << 18);
    GPIOA->PUPDR    |=  (0x5U << 18);          /* pull-up on both */
    GPIOA->AFR[1]   &= ~(0xFFU << 4);          /* clear AF for PA9,10 */
    GPIOA->AFR[1]   |=  (0x77U << 4);          /* AF7 for PA9, PA10 */

    /* Baud rate: USART1 on APB2 = 84 MHz
     * BRR = 84000000 / baudrate
     */
    USART1->BRR = APB2_FREQ / baudrate;

    /* Enable USART, TX, RX */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_send_char(char c)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (uint32_t)c;
}

void uart_send_string(const char *str)
{
    while (*str) {
        uart_send_char(*str++);
    }
}

void uart_send_number(uint32_t num)
{
    char buf[11];
    int i = 10;
    buf[i] = '\0';
    if (num == 0) {
        uart_send_char('0');
        return;
    }
    while (num > 0 && i > 0) {
        buf[--i] = '0' + (num % 10);
        num /= 10;
    }
    uart_send_string(&buf[i]);
}

int uart_receive_char(void)
{
    if (USART1->SR & USART_SR_RXNE) {
        return (int)(USART1->DR & 0xFF);
    }
    return -1;
}

/* Minimal printf using uart_send_char */
/* Redirect printf to UART */
int __io_putchar(int ch)
{
    uart_send_char((char)ch);
    return ch;
}

void uart_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char buf[128];
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (len > 0) {
        for (int i = 0; i < len && buf[i]; i++) {
            uart_send_char(buf[i]);
        }
    }
}
