#ifndef __AHT20_H__
#define __AHT20_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "debug.h"
#include "log.h"
#include "lcd.h"

bool aht20_init(void);
bool aht20_start_measure(void);
bool aht20_wait_for_measure(void);
bool aht20_read_measurement(float *temperture,float *humidity);


#endif/* __AHT20_H__ */
