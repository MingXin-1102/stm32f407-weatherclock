#include "page.h"

void error_page_display(const char *msg)
{
    LCD_Clear(BLACK);
    uint16_t startx = 0;
    int len = strlen(msg) * Font16x24.width ;
    if (len < UI_WIDTH)
        startx = (UI_WIDTH - len + 1)/2;
    LCD_ShowASCIIString(startx, 210, msg, RGB24_TO_RGB565(0xF0, 0x37, 0x38), BLACK, &Font16x24);
    LCD_ShowImage(70, 50, error.width, error.height, error.data);
}
