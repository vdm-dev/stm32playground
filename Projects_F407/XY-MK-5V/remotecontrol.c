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


#include "remotecontrol.h"

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_tim.h>


#define RC_MAX_CHANGES 67


volatile unsigned char g_receivedFlag;
volatile unsigned long g_receivedWord;


void configureRemoteControl(void)
{
    GPIO_InitTypeDef port;

    port.GPIO_Pin = REMOTECONTROL_PIN;
    port.GPIO_Mode = GPIO_Mode_IN;
    port.GPIO_OType = GPIO_OType_OD;
    port.GPIO_PuPd = GPIO_PuPd_NOPULL;
    port.GPIO_Speed = GPIO_High_Speed;
    
    GPIO_Init(REMOTECONTROL_PORT, &port);
    //GPIO_SetBits(REMOTECONTROL_PORT, REMOTECONTROL_PIN);
    
    TIM_TimeBaseInitTypeDef timer;
    
    timer.TIM_Prescaler = 3360;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = 1;
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_RepetitionCounter = 0; // Ignored
   
    TIM_TimeBaseInit(TIM2, &timer);
    TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Repetitive);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
    static uint8_t lastLevel = 0;

    static unsigned long codeWord;
    static unsigned int  timings[RC_MAX_CHANGES];
    static unsigned int  time = 0;
    static unsigned int  lastTime = 0;
    static unsigned char changeCount = 0;    
    static unsigned char repeatCount = 0;
    
    unsigned int  delay;
    unsigned int  delayTolerance;
    unsigned int  duration;
    unsigned char i;    
    
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        time += 160;
        
        if (GPIO_ReadInputDataBit(REMOTECONTROL_PORT, REMOTECONTROL_PIN) == lastLevel)
            return;
        
        lastLevel = GPIO_ReadInputDataBit(REMOTECONTROL_PORT, REMOTECONTROL_PIN);
        
        duration = time - lastTime;
        
        if ((duration > 5000) && (duration > (timings[0] - 200)) && (duration < (timings[0] + 200)))
        {
            ++repeatCount;
            --changeCount;
            
            if (repeatCount == 2)
            {
                codeWord = 0;
                delay = timings[0] / 31;
                delayTolerance = delay * 0.6;
                
                for (i = 1; i < changeCount; i += 2)
                {
                    if ((timings[i] > (delay - delayTolerance)) && (timings[i] < (delay + delayTolerance))
                        && (timings[i + 1] > (delay * 3 - delayTolerance)) && (timings[i + 1] < (delay * 3 + delayTolerance)))
                    {
                        codeWord = codeWord << 1;
                    }
                    else if ((timings[i] > (delay * 3 - delayTolerance)) && (timings[i] < (delay * 3 + delayTolerance))
                        && (timings[i + 1] > (delay - delayTolerance)) && (timings[i + 1] < (delay + delayTolerance)))
                    {
                        ++codeWord;
                        codeWord = codeWord << 1;
                    }
                    else
                    {
                        i = changeCount;
                        codeWord = 0;
                    }
                }
                
                codeWord = codeWord >> 1;
                
                if (!g_receivedFlag && codeWord && (changeCount > 6))
                {
                    g_receivedWord = codeWord;
                    g_receivedFlag = 1;
                }
                
                repeatCount = 0;
            }
            
            changeCount = 0;
        }
        else if (duration > 5000)
        {
            changeCount = 0;
        }
        
        if (changeCount >= RC_MAX_CHANGES)
        {
            changeCount = 0;
            repeatCount = 0;
        }
        
        timings[changeCount] = duration;
        
        ++changeCount;
        
        lastTime = time;
    }
}
