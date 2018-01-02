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


#include "lcd1602.h"

#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>

#include "delay.h"


void LCD1602_WriteFourBits(uint8_t value)
{
    GPIO_ResetBits(LCD1602_RW_PORT, LCD1602_RW_PIN);
    
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    delay(1);
    
    GPIO_SetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    
    GPIO_ResetBits(LCD1602_D_PORT, 0x0F);
    GPIO_SetBits(LCD1602_D_PORT, value & 0x0F);
    delay(1);
    
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    delay(100);
    
    GPIO_ResetBits(LCD1602_D_PORT, 0x0F);
}

void LCD1602_WriteEightBits(uint8_t value)
{
    GPIO_ResetBits(LCD1602_RW_PORT, LCD1602_RW_PIN);
    
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    delay(1);
    
    GPIO_SetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    
    GPIO_ResetBits(LCD1602_D_PORT, 0xFF);    
    GPIO_SetBits(LCD1602_D_PORT, value & 0xFF);
    delay(1);
    
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    delay(100);
    
    GPIO_ResetBits(LCD1602_D_PORT, 0xFF);    
}

void LCD1602_Clear(void)
{
    LCD1602_Command(0x01);
    delay(2000);
}

void LCD1602_Command(uint8_t value)
{
    GPIO_ResetBits(LCD1602_RS_PORT, LCD1602_RS_PIN);
    
#if (LCD1602_DATA_MODE == LCD1602_8BITMODE)
    LCD1602_WriteEightBits(value);
#else
    LCD1602_WriteFourBits(value >> 4);
    LCD1602_WriteFourBits(value);
#endif
}

void LCD1602_SetDisplayMode(uint8_t visible, uint8_t cursor, uint8_t blink)
{
    visible &= 1;
    cursor &= 1;
    blink &= 1;
    
    LCD1602_Command(0x08 | (visible << 2) | (cursor << 1) | blink);
}

void LCD1602_Write(uint8_t value)
{
    GPIO_SetBits(LCD1602_RS_PORT, LCD1602_RS_PIN);
    
#if (LCD1602_DATA_MODE == LCD1602_8BITMODE)
    LCD1602_WriteEightBits(value);
#else
    LCD1602_WriteFourBits(value >> 4);
    LCD1602_WriteFourBits(value);
#endif

    GPIO_ResetBits(LCD1602_RS_PORT, LCD1602_RS_PIN);
}

void LCD1602_WriteString(char* string)
{
    while (*string)
    {
        LCD1602_Write(*string);
        ++string;        
    }
}

void LCD1602_Init(void)
{
    RCC_AHB1PeriphClockCmd(LCD1602_RS_GPIO_CLK | LCD1602_RW_GPIO_CLK | 
                           LCD1602_E_GPIO_CLK | LCD1602_D_GPIO_CLK, ENABLE);
    
    GPIO_InitTypeDef port;

    port.GPIO_Mode = GPIO_Mode_OUT;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd  = GPIO_PuPd_DOWN;
    port.GPIO_Speed = GPIO_High_Speed;
    
    // RS
    port.GPIO_Pin = LCD1602_RS_PIN;
    GPIO_ResetBits(LCD1602_RS_PORT, LCD1602_RS_PIN);
    GPIO_Init(LCD1602_RS_PORT, &port);

    // RW
    port.GPIO_Pin = LCD1602_RW_PIN;
    GPIO_ResetBits(LCD1602_RW_PORT, LCD1602_RW_PIN);
    GPIO_Init(LCD1602_RW_PORT, &port);
    
    // E
    port.GPIO_Pin = LCD1602_E_PIN;
    GPIO_ResetBits(LCD1602_E_PORT, LCD1602_E_PIN);
    GPIO_Init(LCD1602_E_PORT, &port);
    
    // Data
    port.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
#if (LCD1602_DATA_MODE == LCD1602_8BITMODE)
    port.GPIO_Pin |= GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
#endif
    GPIO_ResetBits(LCD1602_D_PORT, port.GPIO_Pin);
    GPIO_Init(LCD1602_D_PORT, &port);
    

#if (LCD1602_DATA_MODE == LCD1602_8BITMODE)
    LCD1602_Command(0x20 | LCD1602_8BITMODE | 0x08);
    delay(4500);

    LCD1602_Command(0x20 | LCD1602_8BITMODE | 0x08);
    delay(150);

    LCD1602_Command(0x20 | LCD1602_8BITMODE | 0x08);
#else

#endif

    LCD1602_Command(0x20 | LCD1602_8BITMODE | 0x08);
    
    LCD1602_SetDisplayMode(1, 0, 0);
    
    LCD1602_Clear();
    
    LCD1602_Command(0x04 | 0x02);
}
