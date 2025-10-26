#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include "rtc.h"

/**
 * @brief 带时间戳的日志输出
 * @param fmt printf 风格格式化字符串
 * @param ... 参数
 */
void log_printf(const char *fmt, ...);

/**
 * @brief SNTP 更新时间后调用该函数校准 RTC
 * @param date_time 从 SNTP 获取到的时间
 */
// void log_sntp_update(rtc_date_time_t *date_time);


#endif /*__LOG_H__*/
