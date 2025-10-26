#ifndef __LED_H
#define __LED_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#if defined(STM32F40_41xxx)
#include "stm32f4xx.h"
typedef GPIO_TypeDef *LED_GPIO_Port_t;
#else
#error led.h: No processor defined!
#endif

typedef struct led_t
{
    LED_GPIO_Port_t port;
    uint32_t pin;
    bool status;
    bool init;
} LED_t;

int LED_Init(LED_t *led, LED_GPIO_Port_t port, uint32_t pin); 
int LED_On(LED_t *led);
int LED_Off(LED_t *led);

#endif /* __LED_H */
