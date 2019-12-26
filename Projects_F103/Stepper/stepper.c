//
//  Copyright (c) 2019 Dmitry Lavygin (vdm.inbox@gmail.com)
// 
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
// 
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
// 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//


#include "stepper.h"

#include <string.h>

#include <stm32f10x.h>

#include "frequency_table.inc"


static volatile bool s_enabled;
static volatile bool s_changed;


void stepperInitialize(void)
{
    s_enabled = false;
    s_changed = false;
    
    GPIO_InitTypeDef port;
    port.GPIO_Speed = GPIO_Speed_50MHz;

    // A2 (TIM2_CH3) Step Output
    port.GPIO_Mode = GPIO_Mode_AF_PP;
    port.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &port);
    
    // A3 (GPIO) Direction Output
    port.GPIO_Mode = GPIO_Mode_Out_PP;
    port.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &port);
    
    TIM_TimeBaseInitTypeDef timer;
    
    timer.TIM_Prescaler = 36000 - 1;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = 0;
    timer.TIM_ClockDivision = TIM_CKD_DIV2;
    timer.TIM_RepetitionCounter = 0;
    
    TIM_TimeBaseInit(TIM2, &timer);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM2_IRQn);

    stepperSetFrequency(0);
    
    TIM_Cmd(TIM2, ENABLE);
}

void stepperSetFrequency(uint16_t frequency)
{
    if (frequency)
    {
        s_enabled = true;
    }
    else
    {
        frequency = 2000;
        s_enabled = false;
    }
    
    s_changed = true;

    TIM_SetAutoreload(TIM2, g_table[(frequency - 1) * 2 + 1] - 1);
    TIM_PrescalerConfig(TIM2, g_table[(frequency - 1) * 2] - 1,
        TIM_PSCReloadMode_Immediate);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        if (!s_enabled)
        {
            GPIOA->BRR = GPIO_Pin_3;
            return;
        }
        
        if (s_changed)
        {
            GPIOA->BRR = GPIO_Pin_3;
            s_changed = false;
            return;
        }
        
        if (GPIOA->ODR & GPIO_Pin_3)
        {
            GPIOA->BRR = GPIO_Pin_3;
        }
        else
        {
            GPIOA->BSRR = GPIO_Pin_3;
        }
    }
}
