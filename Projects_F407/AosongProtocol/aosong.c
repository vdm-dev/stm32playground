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


#include "aosong.h"

#include <stm32f4xx_gpio.h>

#include "delay.h"


void aosongConfigurePort(void)
{
    GPIO_InitTypeDef port;

    port.GPIO_Pin = AOSONG_PIN;
    port.GPIO_Mode = GPIO_Mode_OUT;
    port.GPIO_OType = GPIO_OType_OD;
    port.GPIO_PuPd = GPIO_PuPd_UP;
    port.GPIO_Speed = GPIO_High_Speed;
    
    GPIO_Init(AOSONG_PORT, &port);
    GPIO_SetBits(AOSONG_PORT, AOSONG_PIN);
}

int aosongWait(uint32_t step, uint32_t count, uint8_t state)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        delay(step);
        
        if (GPIO_ReadInputDataBit(AOSONG_PORT, AOSONG_PIN) == state)
            return AOSONG_ERROR_NONE;
    }
    
     if (GPIO_ReadInputDataBit(AOSONG_PORT, AOSONG_PIN) == state)
        return AOSONG_ERROR_NONE;
     
     return AOSONG_ERROR_TIMEOUT;
}

int aosongRead(uint16_t* humidity, int16_t* temperature)
{
    if (!humidity || !temperature)
        return AOSONG_ERROR_INPUT;
    
    *humidity = 0;
    *temperature = 0;
    
    uint8_t checksum = 0;
    //uint8_t validator = 0;
    uint8_t negative = 0;
    
    // Check line
    if ((AOSONG_PORT->IDR & AOSONG_PIN) == 0)
        return AOSONG_ERROR_LINE;
    
    // Start
    GPIO_ResetBits(AOSONG_PORT, AOSONG_PIN);
    delay(2000); // Tbe(typ) * 2
    GPIO_SetBits(AOSONG_PORT, AOSONG_PIN);
    
    // Wait for response (low)
    // Tgo = 20..200 uS
    if (aosongWait(20, 10, Bit_RESET) != AOSONG_ERROR_NONE)
        return AOSONG_ERROR_TIMEOUT;
    
    // Wait for response (high)
    // Trel = 75..85 uS
    if (aosongWait(10, 10, Bit_SET) != AOSONG_ERROR_NONE)
        return AOSONG_ERROR_TIMEOUT;
    
    // Wait for the first bit (low)
    // Treh = 75..85 uS
    if (aosongWait(10, 10, Bit_RESET) != AOSONG_ERROR_NONE)
        return AOSONG_ERROR_TIMEOUT;
    
    // Humidity cycle
    for (uint32_t i = 0; i < 16; ++i)
    {
        // Wait for one bit time
        // Tlow = 48..55 uS
        if (aosongWait(10, 7, Bit_SET) != AOSONG_ERROR_NONE)
            return AOSONG_ERROR_TIMEOUT;
        
        // Th0 = 22..30 uS
        if (aosongWait(2, 20, Bit_RESET) != AOSONG_ERROR_NONE)
        {
            // It's probably BIT(1)
            if (aosongWait(2, 20, Bit_RESET) != AOSONG_ERROR_NONE)
            {
                return AOSONG_ERROR_TIMEOUT;
            }
            else
            {
                *humidity |= (1 << (15 - i));
            }
        }
    }
    
    // Temperature cycle
    for (uint32_t i = 0; i < 16; ++i)
    {
        // Wait for one bit time
        // Tlow = 48..55 uS
        if (aosongWait(10, 7, Bit_SET) != AOSONG_ERROR_NONE)
            return AOSONG_ERROR_TIMEOUT;
        
        // Th0 = 22..30 uS
        if (aosongWait(2, 20, Bit_RESET) != AOSONG_ERROR_NONE)
        {
            // It's probably BIT(1)
            if (aosongWait(2, 20, Bit_RESET) != AOSONG_ERROR_NONE)
            {
                return AOSONG_ERROR_TIMEOUT;
            }
            else
            {
                if (i == 0) // Sign bit
                {
                    negative = 1;
                }
                else
                {
                    *temperature |= (1 << (15 - i));
                }
            }
        }
    }
    
    // Checksum
    for (uint32_t i = 0; i < 8; ++i)
    {
        // Wait for one bit time
        // Tlow = 48..55 uS
        if (aosongWait(10, 7, Bit_SET) != AOSONG_ERROR_NONE)
            return AOSONG_ERROR_TIMEOUT;
        
        // Th0 = 22..30 uS
        if (aosongWait(2, 20, Bit_RESET) != AOSONG_ERROR_NONE)
        {
            // It's probably BIT(1)
            if (aosongWait(2, 20, Bit_RESET) != AOSONG_ERROR_NONE)
            {
                return AOSONG_ERROR_TIMEOUT;
            }
            else
            {
                checksum |= (1 << (7 - i));
            }
        }
    }    
    
    if (negative)
        *temperature *= -1;

    return AOSONG_ERROR_NONE;
}
