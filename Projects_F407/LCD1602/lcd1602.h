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


#ifndef LCD1602_H_INCLUDED
#define LCD1602_H_INCLUDED


#include <stdint.h>


#define LCD1602_RS_PIN                 GPIO_Pin_3
#define LCD1602_RS_PORT                GPIOB
#define LCD1602_RS_GPIO_CLK            RCC_AHB1Periph_GPIOB

#define LCD1602_RW_PIN                 GPIO_Pin_4
#define LCD1602_RW_PORT                GPIOB
#define LCD1602_RW_GPIO_CLK            RCC_AHB1Periph_GPIOB

#define LCD1602_E_PIN                  GPIO_Pin_5
#define LCD1602_E_PORT                 GPIOB
#define LCD1602_E_GPIO_CLK             RCC_AHB1Periph_GPIOB

#define LCD1602_D_PORT                 GPIOD
#define LCD1602_D_GPIO_CLK             RCC_AHB1Periph_GPIOD

#define LCD1602_8BITMODE               0x10
#define LCD1602_4BITMODE               0x00

#define LCD1602_DATA_MODE              LCD1602_8BITMODE


void LCD1602_Clear(void);
void LCD1602_Command(uint8_t value);
void LCD1602_SetDisplayMode(uint8_t visible, uint8_t cursor, uint8_t blink);
void LCD1602_Write(uint8_t value);
void LCD1602_WriteString(char* string);
void LCD1602_Init(void);


#endif // LCD1602_H_INCLUDED
