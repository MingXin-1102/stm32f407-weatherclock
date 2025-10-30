#include "page.h"
void welcome_page_display(void)
{
    extern const uint8_t WELCOME[][72];
    LCD_Clear(BLACK);
    LCD_ShowASCIIString(52, 260, "loading", RGB24_TO_RGB565(0x63, 0x77, 0xC6), BLACK, &Font16x32);
    const uint8_t weather_str[] = {0, 1, 2, 3, 4, 5}; // 智慧天气时钟
    LCD_ShowChineseString(50, 230, weather_str, 6, WELCOME, 24, RGB24_TO_RGB565(0xFE, 0x83, 0x78), BLACK);
    LCD_ShowImage(60, 50, rabbit.width, rabbit.height, rabbit.data);
}
