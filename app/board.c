#include "stm32f4xx.h"
#include "debug.h"
#include "lcd.h"
#include "rtc.h"

void board_init(void)
{
    Debug_Init(115200);
    rtc_init();
    LCD_Init();
}

void model_init(void)
{

}

