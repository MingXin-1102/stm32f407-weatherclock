#include "ui.h"

void welcome_page_display(void)
{
    extern const uint8_t WELCOME[][72];
    LCD_Clear(BLACK);
    LCD_ShowASCIIString(52,260,"loading",RGB24_TO_RGB565(0x63,0x77,0xC6),BLACK,&Font16x32);
    const uint8_t weather_str[] = {0, 1, 2, 3, 4, 5}; // 智慧天气时钟
    LCD_ShowChineseString(50,230, weather_str, 6, WELCOME,24, RGB24_TO_RGB565(0xFE,0x83,0x78), BLACK);
    LCD_ShowImage(60, 50,rabbit.width, rabbit.height,rabbit.data);
}
void error_page_display(void)
{
    LCD_Clear(BLACK);
    LCD_ShowASCIIString(48,210,"error!",RGB24_TO_RGB565(0xF0,0x37,0x38),BLACK,&Font20x40);
    LCD_ShowImage(70, 50,error.width, error.height,error.data);
}
void wifi_page_display(void)
{
    log_printf("[INFO] wifi init ok!\r\n");
    LCD_Clear(BLACK);
    LCD_ShowASCIIString(30,220,"wifi init...",RGB24_TO_RGB565(0x70,0xCF,0xFB),BLACK,&Font16x24);
    LCD_ShowImage(50, 50,wifi.width, wifi.height,wifi.data);
}
void main_page_display(void) //不会改变的主页面
{
    extern const uint8_t INNER[][72];
    extern const uint8_t OUTSIDE[][72];
    extern const uint8_t DATE[][72];
    extern const uint8_t NJ[][72];

    LCD_Clear(BLACK);
    LCD_Fill(10,10,230,130,RGB24_TO_RGB565(0xFE,0xEB,0xB7));
    // const uint8_t date_str[] = {0, 1}; //星期
    LCD_ShowASCIIString(50,35,"--:--",RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font20x50);
    LCD_ShowASCIIString(20,100,"----/--/--",RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font8x16);
    LCD_ShowASCIIString(160,100,"---",RGB24_TO_RGB565(0x5E,0x5D,0x5D), RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font16x24);
    // LCD_ShowChineseString(140,110, date_str, 2, DATE,24, RGB24_TO_RGB565(0x5E,0x5D,0x5D), RGB24_TO_RGB565(0xFE,0xEB,0xB7));
    // LCD_ShowImage(25,18,wifi_mini.width, wifi_mini.height,wifi_mini.data);

    LCD_Fill(10,140,115,310,RGB24_TO_RGB565(0xF0,0xC9,0x93));
    const uint8_t inner_str[] = {0, 1, 2, 3}; // 室内环境
    LCD_ShowChineseString(13,146, inner_str, 4, INNER,24, RGB24_TO_RGB565(0x5E,0x5D,0x5D), RGB24_TO_RGB565(0xF0,0xC9,0x93));
    LCD_ShowASCIIString(32,188,"--",RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xF0,0xC9,0x93),&Font16x32);
    LCD_ShowASCIIString(32,240,"--",RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xF0,0xC9,0x93),&Font16x32);
    // LCD_ShowImage(20,208,temp.width, temp.height,temp.data);

    LCD_Fill(125,140,230,310,RGB24_TO_RGB565(0x93,0xB8,0xF0));
    // const uint8_t outside_str[] = {0, 1, 2, 3}; // 室外环境
    const uint8_t nj_str[] = {0, 1};// 南京
    // LCD_ShowChineseString(128,145, outside_str, 4, OUTSIDE,24, RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0x93,0xB8,0xF0));
    LCD_ShowChineseString(146,142, nj_str, 2, NJ,24, RGB24_TO_RGB565(0x5E,0x5D,0x5D), RGB24_TO_RGB565(0x93,0xB8,0xF0));
    LCD_ShowASCIIString(138,189,"--",RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0x93,0xB8,0xF0), &Font16x32);
    LCD_ShowImage(138,236,temp.width, temp.height,temp.data);
    // LCD_ShowImage(125,162,humi.width, humi.height,humi.data);
}

// void update_time_ui(void)
// {
//     rtc_date_time_t now_time, last_time;
//     rtc_get_time(&now_time); // 从RTC读取当前时间

//     //日期变化时
//     if (now_time.year != last_time.year ||
//         now_time.month != last_time.month ||
//         now_time.day != last_time.day ||
//         now_time.weekday != last_time.weekday)
//     {
//         char date_str[20];
//         char weekday_str[10];
//         sprintf(date_str, "%04d/%02d/%02d",
//                 now_time.year, now_time.month, now_time.day);
//         LCD_ShowASCIIString(20,100,date_str,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font8x16);

//         const char *week_str[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
//         sprintf(weekday_str, "%s ",week_str[now_time.weekday]),
//         log_printf("[parse_sntp_time] : %s \r\n",weekday_str);
//         LCD_ShowASCIIString(160, 100,weekday_str,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font16x24);
//     }

//     // 时间变化时
//     if (now_time.second != last_time.second)
//     {
//         char time_str[16];
//         sprintf(time_str, "%02d:%02d",now_time.hour, now_time.minute);
//         LCD_ShowASCIIString(50,35,time_str,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font20x50);
//     }

//     //保存上次时间
//     memcpy(&last_time, &now_time, sizeof(rtc_date_time_t));
// }

static rtc_date_time_t last_time = {0};
static bool first_run = true;

void update_time_ui(void)
{
    rtc_date_time_t now_time;

    if (!rtc_safe_get_time(&now_time)) {
        log_printf("[ERROR] Failed to get RTC time in update_time_ui\r\n");
        LCD_ShowASCIIString(50,35,"--:--",RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font20x50);
        return;
    }
    // 首次运行或日期变化时更新日期和星期
    if (first_run || now_time.year != last_time.year ||now_time.month != last_time.month ||
        now_time.day != last_time.day ||now_time.weekday != last_time.weekday)
    {
        char date_str[20];
        char weekday_str[10];

        snprintf(date_str, sizeof(date_str), "%04d/%02d/%02d",now_time.year, now_time.month, now_time.day);
        LCD_ShowASCIIString(20, 100, date_str,RGB24_TO_RGB565(0x5E, 0x5D, 0x5D),RGB24_TO_RGB565(0xFE, 0xEB, 0xB7),&Font8x16);

        const char *week_str[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        const char *current_week = "???";
        if (now_time.weekday >= 0 && now_time.weekday < 7) {
            current_week = week_str[now_time.weekday];
        }
        snprintf(weekday_str, sizeof(weekday_str), "%s", current_week);
        LCD_ShowASCIIString(160, 100, weekday_str,RGB24_TO_RGB565(0x5E, 0x5D, 0x5D),RGB24_TO_RGB565(0xFE, 0xEB, 0xB7),&Font16x24);
        log_printf("[update_time_ui] Date updated: %s %s\r\n", date_str, weekday_str);
    }
    // 时间变化时更新时间显示
    if (first_run || now_time.minute != last_time.minute || now_time.hour != last_time.hour)
    {
        char time_str[16];
        snprintf(time_str, sizeof(time_str), "%02d:%02d", now_time.hour, now_time.minute);
        LCD_ShowASCIIString(50, 35, time_str,RGB24_TO_RGB565(0x5E, 0x5D, 0x5D),RGB24_TO_RGB565(0xFE, 0xEB, 0xB7),&Font20x50);
        log_printf("[update_time_ui] Time updated: %s\r\n", time_str);
    }
    // 保存当前时间用于下次比较
    memcpy(&last_time, &now_time, sizeof(rtc_date_time_t));
    first_run = false;
}

extern bool aht20_read_data(float *temperature, float *humidity);
void aht20_display_lcd(float temperature, float humidity)
{
    char temp_buf[32], humi_buf[32];
    sprintf(temp_buf, "%d°", (int)temperature);
    sprintf(humi_buf, "%d%%", (int)humidity);
    LCD_ShowASCIIString(32,188,temp_buf,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xF0,0xC9,0x93),&Font16x32);
    extern const uint8_t DU[];
    LCD_ShowCh(74,188,DU,0,32,128,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xF0,0xC9,0x93));
    LCD_ShowASCIIString(32,240,humi_buf,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0xF0,0xC9,0x93),&Font16x32);
}
const image_t* get_weather_icon(uint8_t code)
{
    switch (code)
    {
        case 0:   // 晴
            return &sunny_0;
        case 1:   // 晴朗
            return &clear_1;
        case 2:   // 晴好
            return &Fair_2;
        case 3:   // 晴好
            return &Fair_3;
        case 4:   // 多云
            return &Cloudy_4;
        case 5:   // 局部多云
            return &Partly_Cloudy_5;
        case 6:   // 局部多云
            return &Partly_Cloudy_6;
        case 7:   // 大部分多云
            return &Mostly_Cloudy_7;
        case 8:   // 大部分多云
            return &Mostly_Cloudy_8;
        case 9:   // 阴天
            return &Overcast_9;
        case 10:  // 阵雨
            return &Shower_10;
        case 11:  // 雷阵雨
            return &Thundershower_11;
        case 12:  // 雷阵雨伴有冰雹
            return &Thundershower_with_Hail_12;
        case 13:  // 小雨
            return &Light_Rain_13;
        case 14:  // 中雨
            return &Moderate_Rain_14;
        case 15:  // 大雨
            return &Heavy_Rain_15;
        case 16:  // 暴雨
            return &Storm_16;
        case 17:  // 大暴雨
            return &Heavy_Storm_17;
        case 18:  // 特大暴雨
            return &Severe_Storm_18;
        case 19:  // 冻雨
            return &Ice_Rain_19;
        case 20:  // 雨夹雪
            return &Sleet_20;
        case 21:  // 小雪
            return &Snow_Flurry_21;
        case 22:  // 小雪
            return &Light_Snow_22;
        case 23:  // 中雪
            return &Moderate_Snow_23;
        case 24:  // 大雪
            return &Heavy_Snow_24;
        case 25:  // 暴雪
            return &Snowstorm_25;
        case 26:  // 浮尘
            return &Dust_26;
        case 27:  // 扬沙
            return &Sand_27;
        case 28:  // 沙尘暴
            return &Duststorm_28;
        case 29:  // 强沙尘暴
            return &Sandstorm_29;
        case 30:  // 雾
            return &Foggy_30;
        case 31:  // 霾
            return &Haze_31;
        case 32:  // 有风
            return &Windy_32;
        case 33:  // 大风
            return &Blustery_33;
        case 34:  // 飓风
            return &Hurricane_34;
        case 35:  // 热带风暴
            return &Tropical_Storm_35;
        case 36:  // 龙卷风
            return &Tornado_36;
        case 37:  // 寒冷
            return &Cold_37;
        case 38:  // 炎热
            return &Hot_38;
        default:  // 默认图标（例如多云）
            return &Cloudy_4;
    }
}
void esp_weather_display_lcd(void)
{
    extern weather_info_t weather_info;
    char temp_buf[32];
    sprintf(temp_buf, "%d",(int) weather_info.temperature);
    LCD_ShowASCIIString(138,188,temp_buf,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0x93,0xB8,0xF0),&Font16x32);
    extern const uint8_t DU[];
    LCD_ShowCh(180,188,DU,0,32,128,RGB24_TO_RGB565(0x5E,0x5D,0x5D),RGB24_TO_RGB565(0x93,0xB8,0xF0));
    // 获取天气图标
    const image_t *image_temp = get_weather_icon(weather_info.weather_code);
    if (image_temp != NULL)
    {
        LCD_ShowImage(168,252, image_temp->width, image_temp->height, image_temp->data);
    }
}
void update_aht20_ui(void)
{
    float temp, humi;
    if (aht20_read_data(&temp, &humi)) {
        // 读取成功，可以显示或处理数据
        aht20_display_lcd(temp, humi);
    }
}

void update_outside_ui(void)
{
    if (esp_at_get_weather())
    {
        esp_weather_display_lcd();
    }
}
void update_wifi_ui(bool connected)
{
    if (connected)
    {
        LCD_ShowASCIIString(113,18,"Minglan_739",RGB24_TO_RGB565(0x5E,0x5D,0x5D), RGB24_TO_RGB565(0xFE,0xEB,0xB7),&Font8x16);
        LCD_ShowImage(25,18,wifi_mini.width, wifi_mini.height,wifi_mini.data);
    }
    else
    {
        LCD_Fill(25,18,25 + wifi_mini.width - 1,18 + wifi_mini.height - 1,RGB24_TO_RGB565(0xFE,0xEB,0xB7));
    }
}
