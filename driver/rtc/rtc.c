#include "rtc.h"

volatile uint32_t rtc_minute_counter = 0; // 分钟计数器
volatile bool flag_weather = false;
volatile bool flag_sntp    = false;
volatile bool flag_aht20   = false;
volatile bool flag_time    = false;

//static int is_lse_ready(void)
//{
//    return (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET);
//}

/* RTC 初始化：按规范顺序配置，包含 EXTI line22 */
void rtc_init(void)
{
    /* 1) 使能 PWR 时钟并允许写入后备域 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    /* 2) 启用 LSE 并等待就绪 */
    RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
        /* 可增加超时判断 */
    }

    /* 3) 选择 LSE 做 RTC 时钟源，打开 RTC 时钟 */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);

    /* 4) RTC 初始化（设置同步/异步分频器，如果需要可以自定义） */
    RTC_InitTypeDef RTC_InitStructure;
    RTC_StructInit(&RTC_InitStructure);
    // RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    /* 使用默认预分频（库会设置合适值）*/
    RTC_Init(&RTC_InitStructure);

    /* 等待同步（读寄存器前最好等待） */
    RTC_WaitForSynchro();

    /* 可选：如果第一次上电需要设置一个初始时间，请在这里调用 rtc_set_time(...) */

    /* 5) 配置 WakeUp 定时器：每 60 秒触发一次
       要点：
         - 先禁能 WakeUp，写保护禁用/启用流程正确
         - 使用 RTC_WakeUpClock_CK_SPRE_16bits (1Hz) 时设置计数器为 60
    */

    /* 禁用写保护 */
    RTC_WriteProtectionCmd(DISABLE);

    /* 先禁用 WakeUp */
    RTC_WakeUpCmd(DISABLE);

    /* 清除可能的中断标志 */
    RTC_ClearITPendingBit(RTC_IT_WUT);

    /* 配置时钟源 - CK_SPRE (1Hz) */
    RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

    /* 设置计数器为 60 -> 每 60 个 tick（1Hz）触发 */
    RTC_SetWakeUpCounter(60);

    /* 使能 WakeUp 中断 */
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    /* 重新启用写保护 */
    RTC_WriteProtectionCmd(ENABLE);

    /* 启用 WakeUp */
    RTC_WakeUpCmd(ENABLE);

    /* 6) EXTI line22 配置（连接 RTC WakeUp 到外部中断线） */
    EXTI_ClearITPendingBit(EXTI_Line22);
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 7) NVIC 配置 */
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
    // date.RTC_WeekDay = 0;
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
        //RTC 防抖处理模式
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

// ==================== RTC 操作层 ====================
/**
 * @brief 验证时间结构体的有效性
 * @param time 要检查的时间结构体
 * @return true-有效 false-无效
 */
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

/**
 * @brief 安全更新RTC时间（带验证）
 * @param time 要设置的时间结构体
 * @return true-成功 false-失败
 */
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

/**
 * @brief 安全获取RTC时间
 * @param time 输出的时间结构体
 * @return true-成功 false-失败
 */
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

// ==================== 时间工具函数 ====================
/**
 * @brief 将时间结构体格式化为字符串
 * @param time 时间结构体
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 格式化后的字符串
 */
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

/**
 * @brief 获取星期几的字符串表示
 * @param weekday 星期几（0-6，0=周日）
 * @return 星期字符串
 */
const char* rtc_get_weekday_string(int weekday)
{
    const char *weekdays[] = {"Sun", "Mon", "Tue", "Wed","Thu", "Fri", "Sat"};
    if (weekday >= 0 && weekday < 7) {
        return weekdays[weekday];
    }
    return "Unknown";
}

/**
 * @brief 获取当前时间字符串（用于显示）
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 格式化后的时间字符串
 */
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

/* 中断处理：必须清 RTC 的 WUT flag，并清 EXTI line22 pending */
void RTC_WKUP_IRQHandler(void)
{
    /* 检查 RTC WakeUp 中断标志 */
    if (RTC_GetITStatus(RTC_IT_WUT) != RESET) {
        /* 清 RTC WUT 中断标志 */
        RTC_ClearITPendingBit(RTC_IT_WUT);

        /* 清 EXTI line 22 pending bit（非常重要）*/
        EXTI_ClearITPendingBit(EXTI_Line22);

        /* 只做标志置位（中断中尽量少做处理）*/
        rtc_minute_counter ++;

        /* 修改为你期望的 10 分钟 / 60 分钟 */
        if (rtc_minute_counter % 1 / 20  == 0) {
            flag_time = true;    // 标记：需要读取 time
        }
        if (rtc_minute_counter % 2 == 0) {
            flag_aht20 = true;    // 标记：需要读取 AHT20
        }
        if (rtc_minute_counter % 3 == 0) {
            flag_weather = true;  // 标记：需要获取天气
        }
        if ((rtc_minute_counter % 1) / 10 == 0) {
            flag_sntp = true;     // 标记：需要 SNTP 校准
        }
    }
}
