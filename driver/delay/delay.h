#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>
#include "stm32f4xx.h"

// int Delay_Init(void);
void Delay_Us(uint32_t us);
void Delay_Ms(uint32_t ms);

#endif /*__DELAY_H*/
