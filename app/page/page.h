#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

#include "font_maple.h"
#include "image.h"
#include "lcd.h"
#include "rtc.h"
#include "weather.h"

#define UI_WIDTH    240
#define UI_HEIGHT   320

void error_page_display(const char *msg);
void welcome_page_display(void);
void wifi_page_display(void);
void main_page_display(void) ;
void main_page_redraw_wifi_ssid(const char *ssid);
void main_page_redraw_time(rtc_date_time_t *time);
void main_page_redraw_date(rtc_date_time_t *date);
void main_page_redraw_inner_temperature(float temperature);
void main_page_redraw_inner_humidity(float humidity);
void main_page_redraw_outdoor_temperature(weather_info_t *weather_info);

#endif  /*__PAGE_H__*/

