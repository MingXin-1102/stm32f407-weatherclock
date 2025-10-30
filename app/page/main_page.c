#include "page.h"

#define COLOR_TEXT         RGB24_TO_RGB565(0x5E, 0x5D, 0x5D)
#define COLOR_BG           RGB24_TO_RGB565(0xFE, 0xEB, 0xB7)
#define POS_TIME_X         50
#define POS_TIME_Y         35
#define POS_DATE_X         20
#define POS_DATE_Y         100
#define POS_WEEK_X         160
#define POS_WEEK_Y         100
#define POS_WIFI_X         25
#define POS_WIFI_Y         18
#define POS_SSID_X         80
#define POS_SSID_Y         18

static const uint16_t color_bg_time = RGB24_TO_RGB565(0xFE, 0xEB, 0xB7);
static const uint16_t color_bg_inner = RGB24_TO_RGB565(0xF0, 0xC9, 0x93);
static const uint16_t color_bg_outside = RGB24_TO_RGB565(0x93, 0xB8, 0xF0);

void main_page_display(void) // 不会改变的主页面
{
    extern const uint8_t INNER[][72];
    extern const uint8_t OUTSIDE[][72];
    extern const uint8_t DATE[][72];
    extern const uint8_t NJ[][72];
    do{
        LCD_Clear(RGB24_TO_RGB565(0x00, 0x00, 0x00));
        LCD_Fill(10, 10, 230, 130, color_bg_time);
        LCD_ShowASCIIString(50, 35, "--:--", RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_inner, &Font20x50);
        LCD_ShowASCIIString(20, 100, "----/--/--", RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_inner, &Font8x16);
        LCD_ShowASCIIString(160, 100, "---", RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_inner, &Font16x24);
    }while(0);

    do{
        LCD_Fill(10, 140, 115, 310, color_bg_inner);
        const uint8_t inner_str[] = {0, 1, 2, 3}; // 室内环境
        LCD_ShowChineseString(13, 146, inner_str, 4, INNER, 24, RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_inner);
        LCD_ShowASCIIString(32, 188, "--", RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_inner, &Font16x32);
        LCD_ShowASCIIString(32, 240, "--", RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_inner, &Font16x32);
    }while(0);

    do{
        LCD_Fill(125, 140, 230, 310, color_bg_outside);
        const uint8_t nj_str[] = {0, 1}; // 南京
        LCD_ShowChineseString(146, 142, nj_str, 2, NJ, 24, RGB24_TO_RGB565(0x5E, 0x5D, 0x5D),color_bg_outside);
        LCD_ShowASCIIString(138, 189, "--", RGB24_TO_RGB565(0x5E, 0x5D, 0x5D), color_bg_outside, &Font16x32);
        LCD_ShowImage(138, 236, temp.width, temp.height, temp.data);
    }while(0);

}

void main_page_redraw_wifi_ssid(const char *ssid)
{
    char str[21];
    uint16_t startx = 0;
    int len = strlen(ssid) * Font8x16.width;
    if (len < UI_WIDTH)
        startx = (UI_WIDTH - len + 1)/2;
    snprintf(str, sizeof(str), "%17s", ssid);
    LCD_ShowImage(POS_WIFI_X, POS_WIFI_Y, wifi_mini.width, wifi_mini.height, wifi_mini.data);
    LCD_ShowASCIIString(startx, POS_SSID_Y,str, COLOR_TEXT, COLOR_BG, &Font8x16);
    log_printf("[UI] WiFi connected\r\n");
}

void main_page_redraw_time(rtc_date_time_t *time)
{
    char str[6];
    char comma = (time->second % 2 == 0) ? ':' : ' ';
    snprintf(str, sizeof(str), "%02u%c%02u", time->hour, comma, time->minute);
    LCD_Fill(POS_TIME_X, POS_TIME_Y, POS_TIME_X + 100, POS_TIME_Y + 50, COLOR_BG);
    LCD_ShowASCIIString(POS_TIME_X, POS_TIME_Y, str, COLOR_TEXT, COLOR_BG, &Font20x50);
    log_printf("[UI] Time updated: %s\r\n", str);
}

void main_page_redraw_date(rtc_date_time_t *date)
{
    char date_str[20];
    snprintf(date_str, sizeof(date_str), "%04d/%02d/%02d",date->year, date->month, date->day);
    LCD_Fill(POS_DATE_X, POS_DATE_Y, POS_DATE_X + 120, POS_DATE_Y + 16, COLOR_BG);
    LCD_ShowASCIIString(POS_DATE_X, POS_DATE_Y, date_str, COLOR_TEXT, COLOR_BG, &Font8x16);
    const char *week_str[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *current_week = (date->weekday < 7) ? week_str[date->weekday] : "???";
    LCD_Fill(POS_WEEK_X, POS_WEEK_Y, POS_WEEK_X + 60, POS_WEEK_Y + 24, COLOR_BG);
    LCD_ShowASCIIString(POS_WEEK_X, POS_WEEK_Y, current_week, COLOR_TEXT, COLOR_BG, &Font16x24);
    log_printf("[UI] Date updated: %s %s\r\n", date_str, current_week);
}

void main_page_redraw_inner_temperature(float temperature)
{
    char temp_buf[32];
    sprintf(temp_buf, "%d", (int)temperature);
    LCD_ShowASCIIString(32, 188, temp_buf, COLOR_TEXT, color_bg_inner, &Font16x32);
    extern const uint8_t DU[];
    LCD_ShowCh(74, 188, DU, 0, 32, 128, COLOR_TEXT, color_bg_inner);
}

void main_page_redraw_inner_humidity(float humidity)
{
    char humi_buf[32];
    sprintf(humi_buf, "%d%%", (int)humidity);
    LCD_ShowASCIIString(32, 240, humi_buf, COLOR_TEXT, color_bg_inner, &Font16x32);
}
static const image_t *get_weather_icon(uint8_t code)
{
    switch (code)
    {
    case 0: // 晴
        return &sunny_0;
    case 1: // 晴朗
        return &clear_1;
    case 2: // 晴好
        return &Fair_2;
    case 3: // 晴好
        return &Fair_3;
    case 4: // 多云
        return &Cloudy_4;
    case 5: // 局部多云
        return &Partly_Cloudy_5;
    case 6: // 局部多云
        return &Partly_Cloudy_6;
    case 7: // 大部分多云
        return &Mostly_Cloudy_7;
    case 8: // 大部分多云
        return &Mostly_Cloudy_8;
    case 9: // 阴天
        return &Overcast_9;
    case 10: // 阵雨
        return &Shower_10;
    case 11: // 雷阵雨
        return &Thundershower_11;
    case 12: // 雷阵雨伴有冰雹
        return &Thundershower_with_Hail_12;
    case 13: // 小雨
        return &Light_Rain_13;
    case 14: // 中雨
        return &Moderate_Rain_14;
    case 15: // 大雨
        return &Heavy_Rain_15;
    case 16: // 暴雨
        return &Storm_16;
    case 17: // 大暴雨
        return &Heavy_Storm_17;
    case 18: // 特大暴雨
        return &Severe_Storm_18;
    case 19: // 冻雨
        return &Ice_Rain_19;
    case 20: // 雨夹雪
        return &Sleet_20;
    case 21: // 小雪
        return &Snow_Flurry_21;
    case 22: // 小雪
        return &Light_Snow_22;
    case 23: // 中雪
        return &Moderate_Snow_23;
    case 24: // 大雪
        return &Heavy_Snow_24;
    case 25: // 暴雪
        return &Snowstorm_25;
    case 26: // 浮尘
        return &Dust_26;
    case 27: // 扬沙
        return &Sand_27;
    case 28: // 沙尘暴
        return &Duststorm_28;
    case 29: // 强沙尘暴
        return &Sandstorm_29;
    case 30: // 雾
        return &Foggy_30;
    case 31: // 霾
        return &Haze_31;
    case 32: // 有风
        return &Windy_32;
    case 33: // 大风
        return &Blustery_33;
    case 34: // 飓风
        return &Hurricane_34;
    case 35: // 热带风暴
        return &Tropical_Storm_35;
    case 36: // 龙卷风
        return &Tornado_36;
    case 37: // 寒冷
        return &Cold_37;
    case 38: // 炎热
        return &Hot_38;
    default: // 默认图标（例如多云）
        return &Cloudy_4;
    }
}

void main_page_redraw_outdoor_temperature(weather_info_t *weather_info)
{
    char temp_buf[32];
    sprintf(temp_buf, "%d", (int)weather_info->temperature);
    LCD_ShowASCIIString(138, 188, temp_buf, COLOR_TEXT, color_bg_outside, &Font16x32);
    extern const uint8_t DU[];
    LCD_ShowCh(180, 188, DU, 0, 32, 128, COLOR_TEXT, color_bg_outside);

    // 获取天气图标
    const image_t *image_temp = get_weather_icon(weather_info->weather_code);
    if (image_temp != NULL)
    {
        LCD_ShowImage(168, 252, image_temp->width, image_temp->height, image_temp->data);
    }
    log_printf("[UI] Weather updated: %d %d %s\r\n", (int)weather_info->temperature,
                weather_info->weather_code,weather_info->city);

}
