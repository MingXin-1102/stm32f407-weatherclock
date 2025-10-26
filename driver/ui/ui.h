#ifndef __UI_H__
#define __UI_H__

#include "lcd.h"
#include "image.h"
#include "rtc.h"
#include "log.h"
#include "esp_at.h"
// #include "weather.h"


void welcome_page_display(void);
void error_page_display(void);
void wifi_page_display(void);
void main_page_display(void) ;

void aht20_display_lcd(float temperature, float humidity);
void esp_weather_display_lcd(void);

void update_time_ui(void);
void update_aht20_ui(void);
void update_outside_ui(void);
void update_wifi_ui(bool connected);

#endif /* __UI_H__ */

