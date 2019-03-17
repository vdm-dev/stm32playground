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

#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_dma.h>


static volatile char s_inputBuffer[IO_UART_INPUT_SIZE];
static volatile char s_outputBuffer[IO_UART_OUTPUT_SIZE];
static volatile bool s_wait = false;

DmaBuffer g_uartChannel;


void configureIoUartPort(void)
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
}

void configureIoUartDma(void)
{
    DMA_InitTypeDef dma;
    
    DMA_StructInit(&dma);
    
    dma.DMA_Channel = DMA_Channel_4;
    dma.DMA_PeripheralBaseAddr = (uint32_t) (&USART2->DR);
    dma.DMA_Memory0BaseAddr = (uint32_t) &s_inputBuffer;
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = IO_UART_INPUT_SIZE;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Circular;
    DMA_Init(DMA1_Stream5, &dma);
    DMA_Cmd(DMA1_Stream5, ENABLE);
    
    dma.DMA_Memory0BaseAddr = (uint32_t) &s_outputBuffer;
    dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma.DMA_BufferSize = 0;
    dma.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Stream6, &dma);
    
    DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
    
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void configureIoUart(void)
{
    configureIoUartPort();
    
    USART_InitTypeDef usart;

    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &usart);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART2, ENABLE);     
    
    configureIoUartDma();
    
    dmaBufferInit(&g_uartChannel, (uint32_t*) &DMA1_Stream5->NDTR,
        (char*) DMA1_Stream5->M0AR, IO_UART_INPUT_SIZE);    
}

bool ioUartReadyToWrite(void)
{
    return !s_wait;
}

void ioUartWrite(const void* data, size_t size)
{
    if (size)
    {
        memmove((void*) &s_outputBuffer, data, size);
        
        s_wait = true;
        DMA_Cmd(DMA1_Stream6, DISABLE);
        DMA1_Stream6->NDTR = size;
        DMA_Cmd(DMA1_Stream6, ENABLE);
    }
}

void ioUartWriteString(const char* string)
{
    ioUartWrite(string, strlen(string));
}

void DMA1_Stream6_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
        s_wait = false;
    }
}
