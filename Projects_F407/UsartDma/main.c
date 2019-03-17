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

#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_dma.h>

#include "io_uart.h"


void configurePeripherals()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

void configureMco()
{
    RCC_MCO2Config(RCC_MCO2Source_SYSCLK, RCC_MCO2Div_4);  

    GPIO_InitTypeDef port;
    
    port.GPIO_Pin = GPIO_Pin_9;
    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd = GPIO_PuPd_NOPULL;
    port.GPIO_Speed = GPIO_High_Speed;

    GPIO_Init(GPIOC, &port);
    GPIO_PinAFConfig(GPIOC, GPIO_Pin_9, GPIO_AF_MCO);    
}

int SER_PutChar (int c)
{
    while(!(USART2->SR & USART_SR_TC)) { }

    USART2->DR = (uint8_t) c;

    return c;
}

void processUartCommand(void)
{
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
        while (size && line[0] != '$')
        {
            line++;
            size--;
        }
        
        if (size < 4)
            continue;
        
        if (strcmp(line, "$HLO") == 0)
        {
            ioUartWriteString("$HAI\r\n");
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
    configurePeripherals();
    configureMco();
    
    configureIoUart();
    
    while (1)
    {
        processUartCommand();
    }
}
