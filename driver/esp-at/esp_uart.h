#ifndef __ESP_UART_H
#define __ESP_UART_H

#include <stdint.h>


typedef void (*esp_usart_receive_callback_t)(uint8_t data);

void esp_uart_init(uint32_t bound_rate);
void esp_uart_recv_callback_register(esp_usart_receive_callback_t callback);
void USART2_SendString(const char *str);

#endif /* __ESP_UART_H */

