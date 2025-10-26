#include "delay.h"

void Delay_Us(uint32_t us)
{
    /* 使能SysTick，时钟源为AHB时钟，即CPU时钟，计数值为us * (CPU频率 / 1000000) - 1 */
    SysTick->LOAD = us * (SystemCoreClock / 1000000) - 1;
    SysTick->VAL = 0;   /* 清空计数值 */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk; /* 使能SysTick，启动计数 */
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0); /* 等待计数完成 */
    SysTick->CTRL = 0;  /* 关闭SysTick */
}
void Delay_Ms(uint32_t ms)
{
    /* 循环调用微妙级延时函数，延时ms毫秒 */
    while (ms--)
    {
        Delay_Us(1000);
    }
}

// void TIM3_Init(uint16_t arr, uint16_t psc)
// {
//     TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

//     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//     TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
//     TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
//     TIM_TimeBaseInitStructure.TIM_Period = arr;
//     TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//     TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
// }

// int Delay_Init(void)
// {
//     TIM3_Init(100 - 1, 84 - 1);
//     return 0;
// }

// void Delay_Us(uint32_t us)
// {
//     TIM3->ARR = us;
//     TIM3->CNT = TIM3->ARR;
//     TIM_Cmd(TIM3, ENABLE);
//     while (TIM3->CNT)
//     {
//         ;
//     }
//     TIM_Cmd(TIM3, DISABLE);
// }

// void Delay_Ms(uint32_t ms)
// {
//     for (int i = 0; i < ms; i++)
//     {
//         Delay_Us(1000);
//     }
// }
