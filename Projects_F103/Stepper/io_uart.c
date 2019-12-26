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


#include "io_uart.h"

#include <string.h>

#include <stm32f10x.h>


#define IO_UART_PORT_BAUD 9600


static volatile char s_inputBuffer[IO_UART_INPUT_SIZE];
static volatile char s_outputBuffer[IO_UART_OUTPUT_SIZE];
static volatile bool s_wait = false;

DmaBuffer g_uartChannel;


void ioUartInitializePort(void)
{
    GPIO_InitTypeDef port;

    port.GPIO_Pin = GPIO_Pin_9;
    port.GPIO_Speed = GPIO_Speed_50MHz;
    port.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &port);

    port.GPIO_Pin = GPIO_Pin_10;
    port.GPIO_Speed = GPIO_Speed_50MHz;
    port.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    GPIO_Init(GPIOA, &port);
}

void ioUartInitializeDma(void)
{
    DMA_InitTypeDef dma;
    
    dma.DMA_PeripheralBaseAddr = (uint32_t) (&USART1->DR);
    dma.DMA_MemoryBaseAddr = (uint32_t) &s_inputBuffer;
    dma.DMA_DIR = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize = IO_UART_INPUT_SIZE;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Circular;
    dma.DMA_Priority = DMA_Priority_High;
    dma.DMA_M2M = DMA_M2M_Disable;
    
    DMA_Init(DMA1_Channel5, &dma);
    DMA_Cmd(DMA1_Channel5, ENABLE);
    
    dma.DMA_MemoryBaseAddr = (uint32_t) &s_outputBuffer;
    dma.DMA_DIR = DMA_DIR_PeripheralDST;
    dma.DMA_BufferSize = 0;
    dma.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel4, &dma);
    
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

void ioUartInitialize(void)
{
    ioUartInitializePort();
    
    USART_InitTypeDef usart;

    usart.USART_BaudRate = IO_UART_PORT_BAUD;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &usart);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1, ENABLE);     
    
    ioUartInitializeDma();
    
    dmaBufferInit(&g_uartChannel, (uint32_t*) &DMA1_Channel5->CNDTR,
        (char*) DMA1_Channel5->CMAR, IO_UART_INPUT_SIZE);    
}

bool ioUartReadyToWrite(void)
{
    return !s_wait;
}

void ioUartWrite(const void* data, size_t size)
{
    if (size && size <= IO_UART_OUTPUT_SIZE)
    {
        memmove((void*) &s_outputBuffer, data, size);
        
        s_wait = true;
        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA1_Channel4->CNDTR = size;
        DMA_Cmd(DMA1_Channel4, ENABLE);
    }
}

void ioUartWriteString(const char* string)
{
    ioUartWrite(string, strlen(string));
}

void DMA1_Channel4_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC4) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        s_wait = false;
    }
}
