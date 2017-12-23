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

#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>

#include <socket.h>
#include <Internet/DHCP/dhcp.h>

#include "delay.h"
#include "w5500_spi.h"


#define SOCK_DHCP     6
#define BUFFER_SIZE   2048


uint8_t g_buffer[BUFFER_SIZE];
wiz_NetInfo g_networkSettings = { .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
                                  .ip = {192, 168, 0, 91},
                                  .sn = {255, 255, 255, 0},
                                  .gw = {192, 168, 0, 1},
                                  .dns = {8, 8, 8, 8},
                                  .dhcp = NETINFO_DHCP };


void configurePeripherals()
{
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

int sendToUsart2(int c)
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
    
    W5500_ConfigureSPI();
    
    if (W5500_Init())
    {
        printf("The system is ready.\r\n");
    }
    else
    {
        printf("Error during W5500 initialization.\r\n");
    }
    


    ctlnetwork(CN_SET_NETINFO, (void*) &g_networkSettings);

    //W5500_PrintNetworkSettings();
    
    DHCP_init(SOCK_DHCP, g_buffer);
    
    uint32_t dhcpTimer = g_sysTick;
    uint8_t attemptCount = 1;
    
    
    while (1)
    {
        // 1ms
        if (g_sysTick - dhcpTimer > 100000)
        {
            DHCP_time_handler();
            
            dhcpTimer = g_sysTick;
        }
        
    	if(g_networkSettings.dhcp == NETINFO_DHCP)
    	{
			switch(DHCP_run())
			{
				case DHCP_IP_ASSIGN:
				case DHCP_IP_CHANGED:
                    printf("DHCP Assign/Update\r\n");
                    W5500_PrintNetworkSettings();
					break;

				case DHCP_IP_LEASED:
                    printf("DHCP Leased\r\n");
                    W5500_PrintNetworkSettings();
                    delay(1000000);
					break;

				case DHCP_FAILED:
					attemptCount++;

                    if (attemptCount > 10)
					{
						g_networkSettings.dhcp = NETINFO_STATIC;
						DHCP_stop();

						printf("DHCP Failed, using static IP\r\n");
                        ctlnetwork(CN_SET_NETINFO, (void*) &g_networkSettings);
						W5500_PrintNetworkSettings();

						attemptCount = 0;
					}
                    else
                    {
						printf("DHCP Failed, attempt %d\r\n", attemptCount);
                    }
					break;
				default:
					break;
			}
    	}        
    }
}
