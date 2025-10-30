#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include "rtc.h"


void log_printf(const char *fmt, ...);
// void log_sntp_update(rtc_date_time_t *date_time);


#endif /*__LOG_H__*/
