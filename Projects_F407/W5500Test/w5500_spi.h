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


#ifndef SPI_H_INCLUDED
#define SPI_H_INCLUDED


#include <stdint.h>


#define W5500_SPI                       SPI1
#define W5500_SPI_CLK                   RCC_APB2Periph_SPI1
#define W5500_SPI_CLK_INIT              RCC_APB2PeriphClockCmd
#define W5500_SPI_IRQn                  SPI1_IRQn
#define W5500_SPI_IRQHANDLER            SPI1_IRQHandler

#define W5500_SCK_PIN                   GPIO_Pin_3
#define W5500_SCK_GPIO_PORT             GPIOB
#define W5500_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB
#define W5500_SCK_SOURCE                GPIO_PinSource3
#define W5500_SCK_AF                    GPIO_AF_SPI1

#define W5500_MISO_PIN                  GPIO_Pin_4
#define W5500_MISO_GPIO_PORT            GPIOB
#define W5500_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define W5500_MISO_SOURCE               GPIO_PinSource4
#define W5500_MISO_AF                   GPIO_AF_SPI1

#define W5500_MOSI_PIN                  GPIO_Pin_5
#define W5500_MOSI_GPIO_PORT            GPIOB
#define W5500_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define W5500_MOSI_SOURCE               GPIO_PinSource5
#define W5500_MOSI_AF                   GPIO_AF_SPI1

#define W5500_NSS_PIN                   GPIO_Pin_15
#define W5500_NSS_GPIO_PORT             GPIOA
#define W5500_NSS_GPIO_CLK              RCC_AHB1Periph_GPIOA
#define W5500_NSS_SOURCE                GPIO_PinSource15
#define W5500_NSS_AF                    GPIO_AF_SPI1


void W5500_ConfigureSPI(void);
uint8_t W5500_Init(void);
void W5500_PrintNetworkSettings(void);


#endif // SPI_H_INCLUDED
