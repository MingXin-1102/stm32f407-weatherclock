#include "log.h"

void log_printf(const char *fmt, ...)
{
    rtc_date_time_t now;
    rtc_get_time(&now);

    printf("[%04d-%02d-%02d %02d:%02d:%02d] ",
           now.year, now.month, now.day,
           now.hour, now.minute, now.second);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


//typedef int __check_rtc_type_defined__[sizeof(rtc_date_time_t)];
// void log_sntp_update(rtc_date_time_t *date_time)
// {
//     if (date_time == NULL) return;
//     rtc_set_time(date_time);
//     log_printf("RTC updated by SNTP\r\n");
// }
