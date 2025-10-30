#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include "stm32f4xx.h"

typedef void (*tim_periodic_callback_t)(void);

void tim_delay_init(void);
uint64_t tim_now(void);
uint64_t tim_get_us(void);
uint64_t tim_get_ms(void);
void tim_delay_us(uint32_t us);
void tim_delay_ms(uint32_t ms);
void tim_register_periodic_callback(tim_periodic_callback_t callback);


// // int Delay_Init(void);
// void Delay_Us(uint32_t us);
// void Delay_Ms(uint32_t ms);

#endif /*__DELAY_H*/
