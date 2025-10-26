#ifndef __RTC_H__
#define __RTC_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "lcd.h"
#include "log.h"

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;  // 1-7, 1=Monday
} rtc_date_time_t;

void rtc_init(void);
void rtc_set_time(const rtc_date_time_t *date_time);
void rtc_get_time(rtc_date_time_t *date_time);
void rtc_printf_time(void);
void rtc_printf_time(void);

bool rtc_validate_time(const rtc_date_time_t *time);
bool rtc_safe_set_time(const rtc_date_time_t *time);
bool rtc_safe_get_time(rtc_date_time_t *time);

void RTC_WKUP_IRQHandler(void);

#endif /* __RTC_H__ */

