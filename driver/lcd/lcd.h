#ifndef __LCD_H_
#define __LCD_H_

#include "stm32f4xx.h"
#include <stdlib.h>
#include <stdint.h>
#include "delay.h"
#include "font.h"
#include "font_maple.h"
// #include "log.h"

#define RGB24_TO_RGB565(r, g, b)    (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
//LCD重要参数集
typedef struct
{
    uint16_t width;            //LCD 宽度
    uint16_t height;            //LCD 高度
    uint16_t id;                //LCD ID
    uint8_t  dir;            //横屏还是竖屏控制：0，竖屏；1，横屏。
    uint16_t  wramcmd;        //开始写gram指令
    uint16_t  setxcmd;        //设置x坐标指令
    uint16_t  setycmd;        //设置y坐标指令
}_lcd_dev_t;
//LCD参数
extern _lcd_dev_t lcddev;   //管理LCD重要参数
//LCD的画笔颜色和背景色
// uint16_t  POINT_COLOR;//默认红色
// uint16_t  BACK_COLOR; //背景颜色.默认为白色

//////////////////////////////////////////////////////////////////////////////////
//-----------------LCD端口定义----------------
//#define    LCD_LED PBout(1)          //LCD背光             PB1
//LCD地址结构体
typedef struct
{
    uint16_t LCD_REG;
    uint16_t LCD_RAM;
} LCD_TypeDef;

//使用NOR/SRAM的 Bank1.sector1,地址位HADDR[27,26]=00   A18作为数据命令区分线
//注意设置时STM32内部会右移一位对其!
#define LCD_BASE        ((uint32_t)(0x60000000 | 0x00007FFFE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////

//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

//画笔颜色
#define WHITE               0xFFFF
#define BLACK               0x0000
#define BLUE                0x001F
#define BRED                0XF81F
#define GRED                0XFFE0
#define GBLUE               0X07FF
#define RED                 0xF800
#define MAGENTA             0xF81F
#define GREEN               0x07E0
#define CYAN                0x7FFF
#define YELLOW              0xFFE0
#define BROWN               0XBC40 //棕色
#define BRRED               0XFC07 //棕红色
#define GRAY                0X8430 //灰色

//GUI颜色
#define DARKBLUE            0X01CF //深蓝色
#define LIGHTBLUE           0X7D7C //浅蓝色
#define GRAYBLUE            0X5458 //灰蓝色
//以上三色为PANEL的颜色

#define LIGHTGREEN          0X841F //浅绿色
#define LIGHTGRAY           0XEF5B //浅灰色(PANNEL)
#define LGRAY               0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE           0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE              0X2B12 //浅棕蓝色(选择条目的反色)


void LCD_Init(void);
void LCD_Display_Dir(uint8_t dir);
void LCD_Clear(uint16_t color);
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);
void LCD_DrawPartImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *img);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode,
                    uint16_t color, uint16_t bgcolor);
void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size,
                    char *p, uint16_t color, uint16_t bgcolor, uint8_t mode);
uint32_t LCD_Pow(uint8_t m,uint8_t n);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size,uint16_t color,uint16_t bgcolor);
// void LCD_ShowxNum2(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);
// // void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p);
void lcd_set_window(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *p);
void LCD_ShowChinese(uint16_t x, uint16_t y, const uint8_t font_lib[][72],
                    uint8_t index,uint8_t size, uint16_t color, uint16_t bgcolor);

void LCD_ShowCh(uint16_t x, uint16_t y, const uint8_t *font_lib, uint8_t index,
                     uint8_t size, uint16_t font_data_size, uint16_t color, uint16_t bgcolor);
void LCD_ShowChString(uint16_t x, uint16_t y, const uint8_t *str_index, uint8_t len,
                          const uint8_t *font_lib, uint8_t size, uint16_t font_data_size,
                          uint16_t color, uint16_t bgcolor);
void LCD_ShowChStringAuto(uint16_t x, uint16_t y, const uint8_t *str_index, uint8_t len,
                              const uint8_t *font_lib, uint8_t size,
                              uint16_t color, uint16_t bgcolor);

void LCD_ShowChineseString(uint16_t x, uint16_t y, const uint8_t *str_index,
                    uint8_t len,const uint8_t font_lib[][72], uint8_t size, uint16_t color, uint16_t bgcolor);

void LCD_ShowASCIIChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color, const FONT_INFO *font);
void LCD_ShowASCIIString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const FONT_INFO *font);
// void LCD_DrawFontChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
// void LCD_DrawFontString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);
// void LCD_ShowASCIIChar(uint16_t x, uint16_t y, char c, uint16_t color, const FONT_INFO *font);
// void LCD_ShowASCIIString(uint16_t x, uint16_t y, const char *str, uint16_t color, const FONT_INFO *font);

void LCD_WR_REG(vu16 regval);
void LCD_WR_DATA(vu16 data);
uint16_t LCD_RD_DATA(void);
void LCD_WriteReg(vu16 LCD_Reg, vu16 LCD_RegValue);
uint16_t LCD_ReadReg(vu16 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
uint16_t LCD_BGR2RGB(uint16_t color);
void opt_delay(uint8_t i);
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_Scan_Dir(uint8_t dir);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t paint_color);
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color);

void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);
//9320/9325 LCD寄存器
#define R0             0x0;
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5

#endif /* __LCD_H_*/

