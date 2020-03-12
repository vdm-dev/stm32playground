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


#include <stdio.h>
#include <stdbool.h>

#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>


#include "delay.h"
#include "remotecontrol.h"


void configurePeripherals()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
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

void configureUsart2()
{
    GPIO_InitTypeDef port;

    port.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd = GPIO_PuPd_UP;
    port.GPIO_Speed = GPIO_High_Speed;

    GPIO_Init(GPIOA, &port);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    
    USART_InitTypeDef usart;

    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);    
}

int SER_PutChar (int c)
{
    while(!(USART2->SR & USART_SR_TC)) { }

    USART2->DR = (uint8_t) c;

    return c;
}

int main()
{
    configurePeripherals();
    configureSysTick();
    configureMco();
    configureUsart2();
    configureRemoteControl();
    
    printf("OK\r\n");
    
    while (true)
    {
        if (g_receivedFlag)
        {
            printf("Received: %u\r\n", g_receivedWord);
            g_receivedFlag = false;
        }
    }
}
