#include "lcd.h"
#include "log.h"
#include <stdio.h>

volatile uint8_t lcd_dma_done = 0;
//管理LCD重要参数,默认为竖屏
_lcd_dev_t lcddev ;

//写寄存器函数,regval:寄存器值
void LCD_WR_REG(vu16 regval)
{
    regval=regval;          //使用-O2优化的时候,必须插入的延时
    LCD->LCD_REG=regval;    //写入要写的寄存器序号
}
//写LCD数据,data:要写入的值
void LCD_WR_DATA(vu16 data)
{
    data=data;              //使用-O2优化的时候,必须插入的延时
    LCD->LCD_RAM=data;
}
//读LCD数据,返回值:读到的值
uint16_t LCD_RD_DATA(void)
{
    vu16 ram;               //防止被优化
    ram=LCD->LCD_RAM;
    return ram;
}
//写寄存器,LCD_Reg:寄存器地址,LCD_RegValue:要写入的数据
void LCD_WriteReg(vu16 LCD_Reg, vu16 LCD_RegValue)
{
    LCD->LCD_REG = LCD_Reg;         //写入要写的寄存器序号
    LCD->LCD_RAM = LCD_RegValue;    //写入数据
}
//读寄存器LCD_Reg:寄存器地址返回值:读到的数据
uint16_t LCD_ReadReg(vu16 LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);            //写入要读的寄存器序号
    tim_delay_us(5);
    return LCD_RD_DATA();           //返回读到的值
}
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
     LCD->LCD_REG=lcddev.wramcmd;
}
//LCD写GRAM,RGB_Code:颜色值
void LCD_WriteRAM(uint16_t RGB_Code)
{
    LCD->LCD_RAM = RGB_Code;        //写十六位GRAM
}
//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。通过该函数转换
//c:GBR格式的颜色值,返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t color)
{
    uint16_t  r,g,b,rgb;
    b=(color>>0)&0x1f;
    g=(color>>5)&0x3f;
    r=(color>>11)&0x1f;
    rgb=(b<<11)+(g<<5)+(r<<0);
    return(rgb);
}
//当mdk -O1时间优化时需要设置,延时i
void opt_delay(uint8_t i)
{
    while(i--);
}
//读取个某点的颜色值,x,y:坐标,返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
    vu16 r = 0, g = 0, b = 0;
    if (x >= lcddev.width || y >= lcddev.height)
        return 0; // 超过了范围,直接返回
    LCD_SetCursor(x, y);
    if (lcddev.id == 0X9341 || lcddev.id == 0X6804 || lcddev.id == 0X5310)
        LCD_WR_REG(0X2E); // 9341/6804/3510 发送读GRAM指令
    else if (lcddev.id == 0X5510)
        LCD_WR_REG(0X2E00); // 5510 发送读GRAM指令
    else
        LCD_WR_REG(R34); // 其他IC发送读GRAM指令
    if (lcddev.id == 0X9320)
        opt_delay(2); // FOR 9320,延时2us
    LCD_RD_DATA();    // dram Read
    opt_delay(2);
    r = LCD_RD_DATA();                                                     // 实际坐标颜色
    if (lcddev.id == 0X9341 || lcddev.id == 0X5310 || lcddev.id == 0X5510) // 9341/NT35310/NT35510要分2次读出
    {
        opt_delay(2);
        b = LCD_RD_DATA();
        g = r & 0XFF; // 对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
        g <<= 8;
    }
    if (lcddev.id == 0X9325 || lcddev.id == 0X4535 || lcddev.id == 0X4531 || lcddev.id == 0XB505 || lcddev.id == 0XC505)
        return r; // 这几种IC直接返回颜色值
    else if (lcddev.id == 0X9341 || lcddev.id == 0X5310 || lcddev.id == 0X5510)
        return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); // ILI9341/NT35310/NT35510需要公式转换一下
    else
        return LCD_BGR2RGB(r); // 其他IC
}
//LCD开启显示
void LCD_DisplayOn(void)
{
    if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X29);    //开启显示
    else if(lcddev.id==0X5510)LCD_WR_REG(0X2900);       //开启显示
    else LCD_WriteReg(R7,0x0173);                       //开启显示
}
//LCD关闭显示
void LCD_DisplayOff(void)
{
    if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD_WR_REG(0X28);    //关闭显示
    else if(lcddev.id==0X5510)LCD_WR_REG(0X2800);    //关闭显示
    else LCD_WriteReg(R7,0x0);//关闭显示
}
//设置光标位置,Xpos:横坐标,Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(Xpos >> 8);
    LCD_WR_DATA(Xpos & 0xFF);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(Ypos >> 8);
    LCD_WR_DATA(Ypos & 0xFF);
    LCD_WR_REG(0x2C);
}
//设置LCD的自动扫描方向,注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h),9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试
void LCD_Scan_Dir(uint8_t dir)
{
    uint16_t regval=0;
    uint16_t dirreg=0;
    uint16_t temp;
    uint16_t xsize,ysize;
    if(lcddev.dir==1&&lcddev.id!=0X6804)    //横屏时，对6804不改变扫描方向！
    {
        switch(dir)//方向转换
        {
            case 0:dir=6;break;
            case 1:dir=7;break;
            case 2:dir=4;break;
            case 3:dir=5;break;
            case 4:dir=1;break;
            case 5:dir=0;break;
            case 6:dir=3;break;
            case 7:dir=2;break;
        }
    }
    if(lcddev.id==0x9341||lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X5510)//9341/6804/5310/5510,很特殊
    {
        switch(dir)
        {
            case L2R_U2D://从左到右,从上到下
                regval|=(0<<7)|(0<<6)|(0<<5);
                break;
            case L2R_D2U://从左到右,从下到上
                regval|=(1<<7)|(0<<6)|(0<<5);
                break;
            case R2L_U2D://从右到左,从上到下
                regval|=(0<<7)|(1<<6)|(0<<5);
                break;
            case R2L_D2U://从右到左,从下到上
                regval|=(1<<7)|(1<<6)|(0<<5);
                break;
            case U2D_L2R://从上到下,从左到右
                regval|=(0<<7)|(0<<6)|(1<<5);
                break;
            case U2D_R2L://从上到下,从右到左
                regval|=(0<<7)|(1<<6)|(1<<5);
                break;
            case D2U_L2R://从下到上,从左到右
                regval|=(1<<7)|(0<<6)|(1<<5);
                break;
            case D2U_R2L://从下到上,从右到左
                regval|=(1<<7)|(1<<6)|(1<<5);
                break;
        }
        if(lcddev.id==0X5510)dirreg=0X3600;
        else dirreg=0X36;
        if((lcddev.id!=0X5310)&&(lcddev.id!=0X5510))regval|=0X08;//5310/5510不需要BGR
        LCD_WriteReg(dirreg,regval);
        if((regval&0X20)||lcddev.dir==1)
        {
            if(lcddev.width<lcddev.height)//交换X,Y
            {
                temp=lcddev.width;
                lcddev.width=lcddev.height;
                lcddev.height=temp;
             }
        }else
        {
            if(lcddev.width>lcddev.height)//交换X,Y
            {
                temp=lcddev.width;
                lcddev.width=lcddev.height;
                lcddev.height=temp;
             }
        }
        if(lcddev.dir==1&&lcddev.id!=0X6804)
        {
            xsize=lcddev.height;
            ysize=lcddev.width;
        }else
        {
            xsize=lcddev.width;
            ysize=lcddev.height;
        }
            LCD_WR_REG(lcddev.setxcmd);
            LCD_WR_DATA(0);LCD_WR_DATA(0);
            LCD_WR_DATA((xsize-1)>>8);LCD_WR_DATA((xsize-1)&0XFF);
            LCD_WR_REG(lcddev.setycmd);
            LCD_WR_DATA(0);LCD_WR_DATA(0);
            LCD_WR_DATA((ysize-1)>>8);LCD_WR_DATA((ysize-1)&0XFF);
      }else{
        switch(dir)
        {
            case L2R_U2D://从左到右,从上到下
                regval|=(1<<5)|(1<<4)|(0<<3);
                break;
            case L2R_D2U://从左到右,从下到上
                regval|=(0<<5)|(1<<4)|(0<<3);
                break;
            case R2L_U2D://从右到左,从上到下
                regval|=(1<<5)|(0<<4)|(0<<3);
                break;
            case R2L_D2U://从右到左,从下到上
                regval|=(0<<5)|(0<<4)|(0<<3);
                break;
            case U2D_L2R://从上到下,从左到右
                regval|=(1<<5)|(1<<4)|(1<<3);
                break;
            case U2D_R2L://从上到下,从右到左
                regval|=(1<<5)|(0<<4)|(1<<3);
                break;
            case D2U_L2R://从下到上,从左到右
                regval|=(0<<5)|(1<<4)|(1<<3);
                break;
            case D2U_R2L://从下到上,从右到左
                regval|=(0<<5)|(0<<4)|(1<<3);
                break;
        }
        LCD_WriteReg(dirreg,regval);
    }
}
//画点,x,y:坐标,POINT_COLOR:此点的颜色
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t paint_color)
{
    LCD_SetCursor(x, y);    // 设置光标位置
    LCD_WriteRAM_Prepare(); // 开始写入GRAM
    LCD->LCD_RAM = paint_color;
}
//快速画点,x,y:坐标,color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
    //设置列地址
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x >> 8);
    LCD_WR_DATA(x & 0xFF);
    LCD_WR_DATA(x >> 8);
    LCD_WR_DATA(x & 0xFF);
    // 设置页地址
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y >> 8);
    LCD_WR_DATA(y & 0xFF);
    LCD_WR_DATA(y >> 8);
    LCD_WR_DATA(y & 0xFF);
    // 写GRAM
    LCD_WR_REG(0x2C);
    LCD_WR_DATA(color >> 8);
    LCD_WR_DATA(color & 0xFF);

    LCD->LCD_REG=lcddev.wramcmd;
    LCD->LCD_RAM=color;
}

//设置LCD显示方向,dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
    if (dir == 0) // 竖屏
    {
        lcddev.dir = 0; // 竖屏
        lcddev.width = 240;
        lcddev.height = 320;
        lcddev.wramcmd = 0X2C;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;

    }
    else // 横屏
    {
        lcddev.dir = 1; // 横屏
        lcddev.width = 320;
        lcddev.height = 240;
        lcddev.wramcmd = 0X2C;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
    }
    LCD_Scan_Dir(DFT_SCAN_DIR); // 默认扫描方向
}
//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy),sx,sy:窗口起始坐标(左上角),width,height:窗口宽度和高度,必须大于0!!,窗体大小:width*height.,68042,横屏时不支持窗口设置!!
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
    width=sx+width-1;
    height=sy+height-1;
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(sx>>8);
    LCD_WR_DATA(sx&0XFF);
    LCD_WR_DATA(width>>8);
    LCD_WR_DATA(width&0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(sy>>8);
    LCD_WR_DATA(sy&0XFF);
    LCD_WR_DATA(height>>8);
    LCD_WR_DATA(height&0XFF);
}
//初始化lcd,该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!在其他型号的驱动芯片上没有测试!
void lcd_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;         // PB1 推挽输出,控制背光
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     // 普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      // 上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);            // 初始化 //PB15 推挽输出,控制背光

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // PD0,1,4,5,8,9,10,14,15 AF OUT
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                                                                                                                                    // 复用输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                                                                                                                                  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;                                                                                                                              // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                                                                                                                                    // 上拉
    GPIO_Init(GPIOD, &GPIO_InitStructure);                                                                                                                                          // 初始化

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // PE7~15,AF OUT
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                                                                                                            // 复用输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                                                                                                          // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;                                                                                                      // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                                                                                                            // 上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);                                                                                                                  // 初始化

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC); //
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC); //
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC); // 和ZET6芯片差异
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC); // 和ZET6芯片差异
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC); //

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC); // PE7,AF12
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC); // PE15,AF12

}
void lcd_fsmc_init(void)
{
    FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef readWriteTiming;
    FSMC_NORSRAMTimingInitTypeDef writeTiming;

    readWriteTiming.FSMC_AddressSetupTime = 0XF; // 地址建立时间（ADDSET）为16个HCLK 1/168M=6ns*16=96ns
    readWriteTiming.FSMC_AddressHoldTime = 0x00; // 地址保持时间（ADDHLD）模式A未用到
    readWriteTiming.FSMC_DataSetupTime = 60;     // 数据保存时间为60个HCLK    =6*60=360ns
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision = 0x00;
    readWriteTiming.FSMC_DataLatency = 0x00;
    readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A; // 模式A

    writeTiming.FSMC_AddressSetupTime = 9;   // 地址建立时间（ADDSET）为9个HCLK =54ns
    writeTiming.FSMC_AddressHoldTime = 0x00; // 地址保持时间（A
    writeTiming.FSMC_DataSetupTime = 8;      // 数据保存时间为6ns*9个HCLK=54ns
    writeTiming.FSMC_BusTurnAroundDuration = 0x00;
    writeTiming.FSMC_CLKDivision = 0x00;
    writeTiming.FSMC_DataLatency = 0x00;
    writeTiming.FSMC_AccessMode = FSMC_AccessMode_A; // 模式A

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;                     //  这里我们使用NE1
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;   // 不复用数据地址
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;              // FSMC_MemoryType_SRAM;  //SRAM
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;     // 存储器数据宽度为16bit
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; // FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable; //  存储器写使能
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; // 读写时序
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;         // 写时序

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); // 初始化FSMC配置
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE); // 使能BANK1

}
void LCD_Init(void)
{
    vu32 i = 0;

    lcd_gpio_init();
    lcd_fsmc_init();

    tim_delay_ms(50); // delay 50 ms
    LCD_WriteReg(0x0000, 0x0001);
    tim_delay_ms(50); // delay 50 ms
    lcddev.id = LCD_ReadReg(0x0000);
    if (lcddev.id < 0XFF || lcddev.id == 0XFFFF || lcddev.id == 0X9300) // 读到ID不正确,新增lcddev.id==0X9300判断，因为9341在未被复位的情况下会被读成9300
    {
        // 尝试9341 ID的读取
        LCD_WR_REG(0XD3);
        lcddev.id = LCD_RD_DATA(); // dummy read
        lcddev.id = LCD_RD_DATA(); // 读到0X00
        lcddev.id = LCD_RD_DATA(); // 读取93
        lcddev.id <<= 8;
        lcddev.id |= LCD_RD_DATA(); // 读取41
    }
    if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510)//如果是这三个IC,则设置WR时序为最快
    {
    //重新配置写时序控制寄存器的时序
    FSMC_Bank1E->BWTR[6] &= ~(0XF << 0);          // 地址建立时间（ADDSET）清零
    FSMC_Bank1E->BWTR[6] &= ~(0XF << 8);          // 数据保存时间清零
    FSMC_Bank1E->BWTR[6] |= 3 << 0;               // 地址建立时间（ADDSET）为3个HCLK =18ns
    FSMC_Bank1E->BWTR[6] |= 2 << 8;               // 数据保存时间为6ns*3个HCLK=18ns
    }
    log_printf("[INFO]lcd id:%x\r\n",lcddev.id); //打印LCD ID
    if(lcddev.id==0X9341)    //9341初始化
    {
        LCD_WR_REG(0xCF);
        LCD_WR_DATA(0x00);
        i++;
        LCD_WR_DATA(0xC1);
        LCD_WR_DATA(0X30);
        LCD_WR_REG(0xED);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0X12);
        LCD_WR_DATA(0X81);
        LCD_WR_REG(0xE8);
        LCD_WR_DATA(0x85);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x7A);
        LCD_WR_REG(0xCB);
        LCD_WR_DATA(0x39);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xF7);
        LCD_WR_DATA(0x20);
        LCD_WR_REG(0xEA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC0);    //Power control
        LCD_WR_DATA(0x1B);   //VRH[5:0]
        LCD_WR_REG(0xC1);    //Power control
        LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0]
        LCD_WR_REG(0xC5);    //VCM control
        LCD_WR_DATA(0x30);      //3F
        LCD_WR_DATA(0x30);      //3C
        LCD_WR_REG(0xC7);    //VCM control2
        LCD_WR_DATA(0XB7);
        LCD_WR_REG(0x36);    // Memory Access Control
        // LCD_WR_DATA(0x48);
        LCD_WR_DATA(0X28);

        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1A);
        LCD_WR_REG(0xB6);    // Display Function Control
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0xA2);
        LCD_WR_REG(0xF2);    // 3Gamma Function Disable
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x26);    //Gamma curve selected
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xE0);    //Set Gamma
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x2A);
        LCD_WR_DATA(0x28);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x0E);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x54);
        LCD_WR_DATA(0XA9);
        LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0XE1);    //Set Gamma
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x15);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x2B);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x0F);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x3f);
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xef);
        LCD_WR_REG(0x11); //Exit Sleep
        tim_delay_ms(120);
        LCD_WR_REG(0x29); //display on
    }
    LCD_Display_Dir(0);             //默认为竖屏
    GPIO_SetBits(GPIOB, GPIO_Pin_1); //点亮背光
    LCD_Clear(WHITE);
}
//清屏函数，color:要清屏的填充色
void LCD_Clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height;                    // 得到总点数

    LCD_SetCursor(0x00, 0x0000);                    // 设置光标位置
    LCD_Set_Window(0, 0, lcddev.width-1, lcddev.height-1);
    LCD_WriteRAM_Prepare();                         // 开始写入GRAM
    for (index = 0; index < totalpoint; index++)
    {
        LCD->LCD_RAM = color;
    }
}

void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{
    uint16_t i,j;
    uint16_t xlen=ex-sx+1;

    // 设置整个区域一次
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(sx >> 8);
    LCD_WR_DATA(sx & 0xFF);
    LCD_WR_DATA(ex >> 8);
    LCD_WR_DATA(ex & 0xFF);

    LCD_WR_REG(0x2B);
    LCD_WR_DATA(sy >> 8);
    LCD_WR_DATA(sy & 0xFF);
    LCD_WR_DATA(ey >> 8);
    LCD_WR_DATA(ey & 0xFF);

    LCD_WR_REG(0x2C);  // 开始写GRAM

    // 连续写入像素
    for (i = sy; i <= ey; i++)
    {
        for (j = 0; j < xlen; j++)
        {
            LCD->LCD_RAM = color;
        }
    }
}

//在指定区域内填充指定颜色块，(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)，color:要填充的颜色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{
    uint16_t height,width;
    uint16_t i,j;
    width=ex-sx+1;             //得到填充的宽度
    height=ey-sy+1;            //高度
     for(i=0;i<height;i++)
    {
        LCD_SetCursor(sx,sy+i);       //设置光标位置
        LCD_WriteRAM_Prepare();     //开始写入GRAM
        for(j=0;j<width;j++)LCD->LCD_RAM=color[i*width+j];//写入数据
    }
}
//画指定区域图像
void LCD_DrawPartImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *img)
{
    const uint8_t *p = img;
    for (uint16_t j = 0; j < h; j++)
    {
        LCD_SetCursor(x, y + j);
        LCD_WriteRAM_Prepare();
        for (uint16_t i = 0; i < w; i++)
        {
            uint16_t color = (p[0] << 8) | p[1];  // 高字节在前
            LCD_WR_DATA(color);
            p += 2;
        }
    }
}

//画线，x1,y1:起点坐标，x2,y2:终点坐标
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; // 计算坐标增量
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0)
        incx = 1; // 设置单步方向
    else if (delta_x == 0)
        incx = 0; // 垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0; // 水平线
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)
        distance = delta_x; // 选取基本增量坐标轴
    else
        distance = delta_y;
    for (t = 0; t <= distance + 1; t++) // 画线输出
    {
        LCD_DrawPoint(uRow, uCol, color); // 画点
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

//画矩形，(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
    LCD_DrawLine(x1,y1,x2,y1,color);
    LCD_DrawLine(x1,y1,x1,y2,color);
    LCD_DrawLine(x1,y2,x2,y2,color);
    LCD_DrawLine(x2,y1,x2,y2,color);
}

//在指定位置画一个指定大小的圆，(x,y):中心点，r  :半径
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1); // 判断下个点位置的标志
    while (a <= b)
    {
        LCD_DrawPoint(x0 + a, y0 - b, color); // 5
        LCD_DrawPoint(x0 + b, y0 - a, color); // 0
        LCD_DrawPoint(x0 + b, y0 + a, color); // 4
        LCD_DrawPoint(x0 + a, y0 + b, color); // 6
        LCD_DrawPoint(x0 - a, y0 + b, color); // 1
        LCD_DrawPoint(x0 - b, y0 + a, color);
        LCD_DrawPoint(x0 - a, y0 - b, color); // 2
        LCD_DrawPoint(x0 - b, y0 - a, color); // 7
        a++;
        // 使用Bresenham算法画圆
        if (di < 0)
            di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b --;
        }
    }
}

// 在指定位置显示一个字符，x,y:起始坐标，num:要显示的字符:" "--->"~"，size:字体大小 12/16/24，mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint8_t size, uint8_t mode,
                    uint16_t color, uint16_t bgcolor)
{
    // printf("LCD_ShowChar chr=%c size=%d num=%d\r\n", chr, size, chr - ' ');

    uint8_t temp, t, t1;
    uint16_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); //如果size不能被8整除（即有余数），则加1，否则加0
    uint8_t num = chr - ' ';

    for (t = 0; t < csize; t++) //csize 3+0 *12= 36
    {
        if (size == 12)
            temp = asc2_1206[num][t];
        else if (size == 16)
            temp = asc2_1608[num][t];
        else if (size == 24)
        {
            temp = asc2_2412[num][t];
            // printf("LCD_ShowChar temp=%d \r\n", temp);
        }
        else
            return;

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                LCD_Fast_DrawPoint(x, y, color);
            else if (mode == 0)
                LCD_Fast_DrawPoint(x, y, bgcolor);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size,
                    char *p, uint16_t color, uint16_t bgcolor, uint8_t mode)
{
    while (*p != '\0')
    {
        if (x > (width - size / 2))
        {
            x = 0;
            y += size;
        }
        // if (y > height)
        if ( y < height)
            break;

        LCD_ShowChar(x, y, *p, size, mode, color, bgcolor);
        x += size / 2;
        p++;
    }
    // printf("LCD_ShowChar chr=%c \r\n", p-1);
}

//m^n函数,返回值:m^n次方.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;
    return result;
}
//显示数字,高位为0,则不显示,x,y :起点坐标,len :数字的位数,size:字体大小,color:颜色,num:数值(0~4294967295);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size,uint16_t color,uint16_t bgcolor)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + (size / 2) * t, y, ' ', size,0,color,bgcolor);
                continue;
            }
            else
                enshow = 1;
        }
        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size,0,color,bgcolor);
    }
}


void lcd_set_window(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    // 设置列（X坐标）范围
    LCD_WR_REG(0x2A);  // Column Address Set
    LCD_WR_DATA(x_start >> 8);
    LCD_WR_DATA(x_start & 0xFF);
    LCD_WR_DATA(x_end >> 8);
    LCD_WR_DATA(x_end & 0xFF);

    // 设置行（Y坐标）范围
    LCD_WR_REG(0x2B);  // Page Address Set
    LCD_WR_DATA(y_start >> 8);
    LCD_WR_DATA(y_start & 0xFF);
    LCD_WR_DATA(y_end >> 8);
    LCD_WR_DATA(y_end & 0xFF);

    // 写入下一条命令：准备写像素数据
    LCD_WR_REG(0x2C);  // Memory Write
}

void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *p)
{
    uint32_t total = width * height;

    // 设置显示窗口
    LCD_WR_REG(0x2A); // 列地址设置
    LCD_WR_DATA(x >> 8);
    LCD_WR_DATA(x & 0xFF);
    LCD_WR_DATA((x + width - 1) >> 8);
    LCD_WR_DATA((x + width - 1) & 0xFF);

    LCD_WR_REG(0x2B); // 行地址设置
    LCD_WR_DATA(y >> 8);
    LCD_WR_DATA(y & 0xFF);
    LCD_WR_DATA((y + height - 1) >> 8);
    LCD_WR_DATA((y + height - 1) & 0xFF);

    // 写入GRAM
    LCD_WR_REG(0x2C);
    for (uint32_t i = 0; i < total; i++)
    {
        uint16_t color = (p[2 * i] << 8) | p[2 * i + 1]; // RGB565: 高字节在前
        LCD->LCD_RAM = color;
    }

    // 强制退出 Memory Write 状态
    LCD_WR_REG(0x00);

    // 重新设置默认窗口（防止下次绘制被锁定）
    LCD_SetCursor(0, 0);
}

void LCD_ShowChinese(uint16_t x, uint16_t y,const uint8_t font_lib[][72],uint8_t index, uint8_t size,
                     uint16_t color, uint16_t bgcolor)
{
    const uint8_t *p = font_lib[index];
    uint8_t bytes_per_row;
    if (size == 24)
        bytes_per_row = 3;
    else if (size == 16)
        bytes_per_row = 2;
    else if (size == 12)
        bytes_per_row = 2;
    else
        bytes_per_row = (size + 7) / 8;

    for (uint8_t row = 0; row < size; row++)
    {
        for (uint8_t col_byte = 0; col_byte < bytes_per_row; col_byte++)
        {
            uint8_t temp = p[row * bytes_per_row + col_byte];
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                if (col_byte * 8 + bit >= size)
                    break;
                if (temp & 0x80)
                    LCD_Fast_DrawPoint(x + col_byte * 8 + bit, y + row, color);
                else
                    LCD_Fast_DrawPoint(x + col_byte * 8 + bit, y + row, bgcolor);
                temp <<= 1;
            }
        }
    }
}
void LCD_ShowChineseString(uint16_t x, uint16_t y, const uint8_t *str_index, uint8_t len,const uint8_t font_lib[][72], uint8_t size, uint16_t color, uint16_t bgcolor)
{
    for (uint8_t i = 0; i < len; i++)
    {
        LCD_ShowChinese(x + i * size , y, font_lib, str_index[i],size, color, bgcolor);
    }
}

void LCD_ShowCh(uint16_t x, uint16_t y, const uint8_t *font_lib, uint8_t index,
                     uint8_t size, uint16_t font_data_size, uint16_t color, uint16_t bgcolor)
{
    // 计算字符数据在字库中的偏移量
    const uint8_t *p = font_lib + (index * font_data_size);

    // 计算每行字节数
    uint8_t bytes_per_row = (size + 7) / 8; // 向上取整计算每行需要的字节数

    // 遍历每一行
    for (uint8_t row = 0; row < size; row++)
    {
        // 遍历当前行的每个字节
        for (uint8_t col_byte = 0; col_byte < bytes_per_row; col_byte++)
        {
            uint8_t temp = p[row * bytes_per_row + col_byte];

            // 遍历当前字节的每个位
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                // 检查是否超出字体宽度
                if (col_byte * 8 + bit >= size)
                    break;

                // 根据位值设置像素颜色
                if (temp & 0x80)
                    LCD_Fast_DrawPoint(x + col_byte * 8 + bit, y + row, color);
                else
                    LCD_Fast_DrawPoint(x + col_byte * 8 + bit, y + row, bgcolor);

                temp <<= 1;
            }
        }
    }
}

void LCD_ShowChString(uint16_t x, uint16_t y, const uint8_t *str_index, uint8_t len,
                          const uint8_t *font_lib, uint8_t size, uint16_t font_data_size,
                          uint16_t color, uint16_t bgcolor)
{
    for (uint8_t i = 0; i < len; i++)
    {
        LCD_ShowCh(x + i * size, y, font_lib, str_index[i],
                       size, font_data_size, color, bgcolor);
    }
}

void LCD_ShowChStringAuto(uint16_t x, uint16_t y, const uint8_t *str_index, uint8_t len,
                              const uint8_t *font_lib, uint8_t size,
                              uint16_t color, uint16_t bgcolor)
{
    uint16_t font_data_size;

    switch(size) {
        case 12:
            font_data_size = 24; // 12x12, 每行2字节，共12行 = 24字节
            break;
        case 16:
            font_data_size = 32; // 16x16, 每行2字节，共16行 = 32字节
            break;
        case 24:
            font_data_size = 72; // 24x24, 每行3字节，共24行 = 72字节
            break;
        case 32:
            font_data_size = 128; // 32x32, 每行4字节，共32行 = 128字节
            break;
        default:
            // 对于其他尺寸，自动计算：每行字节数 = (size + 7) / 8，总字节数 = 每行字节数 * size
            font_data_size = ((size + 7) / 8) * size;
            break;
    }

    LCD_ShowChString(x, y, str_index, len, font_lib, size, font_data_size, color, bgcolor);
}

void LCD_ShowASCIIChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg_color, const FONT_INFO *font)
{
    if (c < 32 || c > 126) return;

    uint16_t index = c - 32;
    uint8_t bytes_per_row = (font->width + 7) / 8;
    uint32_t bytes_per_char = font->height * bytes_per_row;
    const uint8_t *p = font->table + index * bytes_per_char;

    for (uint16_t i = 0; i < font->height; i++)  // 行
    {
        for (uint16_t j = 0; j < font->width; j++) // 列
        {
            uint8_t byte = p[i * bytes_per_row + j / 8];
            uint8_t bit = 0x80 >> (j % 8);
            uint16_t color_to_draw = (byte & bit) ? color : bg_color;
            LCD_DrawPoint(x + j, y + i, color_to_draw);
        }
    }
}

void LCD_ShowASCIIString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const FONT_INFO *font)
{
    if (!str || !font) return;

    //uint16_t orig_x = x;
    uint16_t str_len = strlen(str);
    uint16_t total_width = str_len * font->width;
    uint16_t total_height = font->height;

    // 先擦除背景区域，防止文字重叠
    LCD_Fill(x, y, x + total_width - 1, y + total_height - 1, bg_color);

    // 逐字符显示
    while (*str)
    {
        LCD_ShowASCIIChar(x, y, *str, color, bg_color, font);
        x += font->width;
        str++;
    }
}

