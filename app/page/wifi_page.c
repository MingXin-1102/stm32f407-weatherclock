#include "page.h"

void wifi_page_display(void)
{
    LCD_Clear(BLACK);
    LCD_ShowASCIIString(30, 220, "wifi init...", RGB24_TO_RGB565(0x70, 0xCF, 0xFB), BLACK, &Font16x24);
    LCD_ShowImage(50, 50, wifi.width, wifi.height, wifi.data);
}

