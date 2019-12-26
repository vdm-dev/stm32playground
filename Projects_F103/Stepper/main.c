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


#include <stdio.h>
#include <string.h>

#include <stm32f10x.h>

#include "io_uart.h"
#include "stepper.h"


void hardwareInitialize(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    GPIO_InitTypeDef port;

    port.GPIO_Pin = GPIO_Pin_13;
    port.GPIO_Speed = GPIO_Speed_50MHz;
    port.GPIO_Mode = GPIO_Mode_Out_OD;

    GPIO_Init(GPIOC, &port);    

    //GPIOC->BSRR = GPIO_Pin_13;
}

void processUartCommand(void)
{
    static uint16_t f = 0;
    
    if (!ioUartReadyToWrite())
        return;
    
    char buffer[100];
    
    for (;;)
    {
        size_t size = dmaBufferGetLine(&g_uartChannel, buffer, sizeof(buffer));
        
        if (size == 0)
        {
             if (dmaBufferBytesUsed(&g_uartChannel) >= sizeof(buffer))
                 dmaBufferClear(&g_uartChannel);
             
             break;
        }
        
        char* line = buffer;
        
        // Trim end
        while (size && (line[size - 1] == '\r' || line[size - 1] == '\n'))
        {
            size--;
            line[size] = 0;
        }
        
        // Trim beginning
        /*
        while (size && line[0] != '$')
        {
            line++;
            size--;
        }
        */
        
        if (size < 1)
            continue;
        
        if (strcmp(line, "f") == 0)
        {
            stepperSetFrequency(f);
            f++;
            if (f > 2000)
            {
                f = 0;
            }
            
            ioUartWriteString("F OK\r\n");
        }
        else
        {
            continue;
        }
        
        break;
    }
}

int main()
{
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0x08)
        SystemInit();
    
    hardwareInitialize();
    ioUartInitialize();
    stepperInitialize();
    
    ioUartWriteString("Hello\r\n");
    
    while (1)
    {
        processUartCommand();
    }
}
