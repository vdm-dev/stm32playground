//
//  Copyright (c) 2017 Dmitry Lavygin (vdm.inbox@gmail.com)
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


#include <stm32f4xx.h>


int main()
{
    RCC_MCO2Config(RCC_MCO2Source_SYSCLK, RCC_MCO2Div_4);  
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    
    GPIO_InitTypeDef port;
    
    // MCO
    port.GPIO_Pin = GPIO_Pin_9;
    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd = GPIO_PuPd_NOPULL;
    port.GPIO_Speed = GPIO_High_Speed;
    
    GPIO_Init(GPIOC, &port);
    GPIO_PinAFConfig(GPIOC, GPIO_Pin_9, GPIO_AF_MCO);
    
    // TIM2 & TIM3 CH
    port.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_6;
    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd = GPIO_PuPd_NOPULL;
    port.GPIO_Speed = GPIO_High_Speed;    

    GPIO_Init(GPIOA, &port);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);

    // TIM4_CH
    port.GPIO_Pin = GPIO_Pin_6;
    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd = GPIO_PuPd_NOPULL;
    port.GPIO_Speed = GPIO_High_Speed;  

    GPIO_Init(GPIOB, &port);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);

   
    TIM_TimeBaseInitTypeDef timer;
    TIM_OCInitTypeDef timerOutput;

    // TIM2    
    timer.TIM_Prescaler = 0;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = 839;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_RepetitionCounter = 0; // Ignored
   
    TIM_TimeBaseInit(TIM2, &timer);
    
    
    timerOutput.TIM_OCMode = TIM_OCMode_PWM2;
    timerOutput.TIM_OutputState = TIM_OutputState_Enable;
    timerOutput.TIM_Pulse = 420;
    timerOutput.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC1Init(TIM2, &timerOutput);

    // TIM3
    timer.TIM_Prescaler = 0;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = 251;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_RepetitionCounter = 0; // Ignored
   
    TIM_TimeBaseInit(TIM3, &timer);
    
    
    timerOutput.TIM_OCMode = TIM_OCMode_PWM2;
    timerOutput.TIM_OutputState = TIM_OutputState_Enable;
    timerOutput.TIM_Pulse = 168;
    timerOutput.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC1Init(TIM3, &timerOutput);
    TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);


    TIM_SelectInputTrigger(TIM3, TIM_TS_ITR1);

    TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);

    // TIM4
    timer.TIM_Prescaler = 0;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = 671;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_RepetitionCounter = 0; // Ignored
   
    TIM_TimeBaseInit(TIM4, &timer);
    
    
    timerOutput.TIM_OCMode = TIM_OCMode_PWM2;
    timerOutput.TIM_OutputState = TIM_OutputState_Enable;
    timerOutput.TIM_Pulse = 588;
    timerOutput.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC1Init(TIM4, &timerOutput);
    TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);


    TIM_SelectInputTrigger(TIM4, TIM_TS_ITR1);

    TIM_SelectMasterSlaveMode(TIM4, TIM_MasterSlaveMode_Enable);
    TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Trigger);

    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

    TIM_SetCounter(TIM2, 0);
    TIM_SetCounter(TIM3, 0);
    TIM_SetCounter(TIM4, 0);

    TIM_Cmd(TIM2, ENABLE);

    while (1)
    {
    }
}
