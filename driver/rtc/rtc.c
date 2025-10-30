#include "rtc.h"

volatile uint32_t rtc_minute_counter = 0; // 分钟计数器
// volatile bool flag_weather = false;
// volatile bool flag_sntp    = false;
// volatile bool flag_aht20   = false;
// volatile bool flag_time    = false;

//static int is_lse_ready(void)
//{
//    return (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET);
//}

void rtc_init(void)
{
    RCC_RTCCLKCmd(ENABLE);

    RTC_InitTypeDef RTC_InitStructure;
    RTC_StructInit(&RTC_InitStructure);
    RTC_Init(&RTC_InitStructure);

    RTC_WaitForSynchro();
    RTC_WriteProtectionCmd(DISABLE);

    RTC_WakeUpCmd(DISABLE);

    RTC_ClearITPendingBit(RTC_IT_WUT);
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
    RTC_SetWakeUpCounter(60);

    RTC_ITConfig(RTC_IT_WUT, ENABLE);
    RTC_WriteProtectionCmd(ENABLE);
    RTC_WakeUpCmd(ENABLE);

    /* 6) EXTI line22 配置（连接 RTC WakeUp 到外部中断线） */
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
static void _rtc_set_time_once(const rtc_date_time_t *date_time)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    RTC_DateStructInit(&date);
    RTC_TimeStructInit(&time);

    date.RTC_Year = date_time->year - 2000;
    date.RTC_Month = date_time->month;
    date.RTC_Date = date_time->day;
    date.RTC_WeekDay = date_time->weekday;
    time.RTC_Hours = date_time->hour;
    time.RTC_Minutes = date_time->minute;
    time.RTC_Seconds = date_time->second;
    time.RTC_H12 = 0;

    RTC_SetTime(RTC_Format_BIN, &time);
    RTC_SetDate(RTC_Format_BIN, &date);
}
static void _rtc_get_time_once(rtc_date_time_t *date_time)
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    RTC_DateStructInit(&date);
    RTC_TimeStructInit(&time);

    RTC_GetTime(RTC_Format_BIN, &time);
    RTC_GetDate(RTC_Format_BIN, &date);

    date_time->year = 2000 + date.RTC_Year;
    date_time->month = date.RTC_Month;
    date_time->day = date.RTC_Date;
    date_time->weekday = date.RTC_WeekDay ;
    date_time->hour = time.RTC_Hours;
    date_time->minute = time.RTC_Minutes;
    date_time->second = time.RTC_Seconds;
}
void rtc_set_time(const rtc_date_time_t *date_time)
{
    rtc_date_time_t rtime;
    do {
        _rtc_set_time_once(date_time);
        _rtc_get_time_once(&rtime);
    } while (date_time->second != rtime.second);
}
void rtc_get_time(rtc_date_time_t *date_time)
{
    rtc_date_time_t time1, time2;
    do {
        _rtc_get_time_once(&time1);
        _rtc_get_time_once(&time2);
    } while (memcmp(&time1,&time2,sizeof(rtc_date_time_t)) != 0);

    memcpy(date_time, &time1, sizeof(rtc_date_time_t));
}
void rtc_printf_time(void)
{
    rtc_date_time_t now;
    rtc_get_time(&now);

    printf("[%04d-%02d-%02d %02d:%02d:%02d] ",
        now.year,
        now.month,
        now.day,
        now.hour,
        now.minute,
        now.second);
}
bool rtc_validate_time(const rtc_date_time_t *time)
{
    if (time == NULL) {
        return false;
    }

    if (time->year < 2000 || time->year > 2100) return false;
    if (time->month < 1 || time->month > 12) return false;
    if (time->day < 1 || time->day > 31) return false;
    if (time->hour > 23) return false;
    if (time->minute > 59) return false;
    if (time->second > 59) return false;
    if (time->weekday > 6) return false;

    const int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max_days = days_in_month[time->month - 1];

    if (time->month == 2) {
        int is_leap = (time->year % 4 == 0 && time->year % 100 != 0) || (time->year % 400 == 0);
        if (is_leap) max_days = 29;
    }
    if (time->day > max_days) return false;

    return true;
}
bool rtc_safe_set_time(const rtc_date_time_t *time)
{
    if (!rtc_validate_time(time)) {
        log_printf("[ERROR] RTC set time failed: invalid time\r\n");
        return false;
    }

    rtc_set_time(time);

    log_printf("[INFO] RTC set successfully: %04d-%02d-%02d %02d:%02d:%02d\r\n",
               time->year, time->month, time->day,
               time->hour, time->minute, time->second);
    return true;
}

bool rtc_safe_get_time(rtc_date_time_t *time)
{
    if (time == NULL) {
        return false;
    }

    rtc_get_time(time);

    if (!rtc_validate_time(time)) {
        log_printf("[WARNING] RTC get time: time validation failed\r\n");
        return false;
    }

    return true;
}

const char* rtc_format_time(const rtc_date_time_t *time, char *buffer, size_t buffer_size)
{
    if (time == NULL || buffer == NULL || buffer_size < 20) {
        return "Invalid time";
    }

    snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d",
             time->year, time->month, time->day,
             time->hour, time->minute, time->second);

    return buffer;
}

const char* rtc_get_weekday_string(int weekday)
{
    const char *weekdays[] = {"Sun", "Mon", "Tue", "Wed","Thu", "Fri", "Sat"};
    if (weekday >= 0 && weekday < 7) {
        return weekdays[weekday];
    }
    return "Unknown";
}

const char* rtc_get_current_time_string(char *buffer, size_t buffer_size)
{
    rtc_date_time_t current_time;

    if (rtc_safe_get_time(&current_time)) {
        return rtc_format_time(&current_time, buffer, buffer_size);
    } else {
        if (buffer && buffer_size > 0) {
            strncpy(buffer, "Time unavailable", buffer_size);
            buffer[buffer_size - 1] = '\0';
        }
        return buffer;
    }
}

void RTC_WKUP_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        RTC_ClearITPendingBit(RTC_IT_WUT);
        EXTI_ClearITPendingBit(EXTI_Line22);

        rtc_minute_counter ++;
        // if (rtc_minute_counter % 1  == 0) {
        //     flag_time = true;    // 标记：需要读取 time
        // }
        // if (rtc_minute_counter % 2 == 0) {
        //     flag_aht20 = true;    // 标记：需要读取 AHT20
        // }
        // if (rtc_minute_counter % 5 == 0) {
        //     flag_weather = true;  // 标记：需要获取天气
        // }
        // if (rtc_minute_counter % 60 == 0) {
        //     flag_sntp = true;     // 标记：60 分钟需要 SNTP 校准
        // }
    }
}
