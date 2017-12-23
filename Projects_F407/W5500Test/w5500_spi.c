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


#include "w5500_spi.h"

#include <string.h>
#include <stdio.h>

#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_spi.h>

#include <socket.h>


// Callback Functions
static void W5500_ChipSelect(void)
{
    GPIO_ResetBits(W5500_NSS_GPIO_PORT, W5500_NSS_PIN);
}

static void W5500_ChipDeselect(void)
{
    GPIO_SetBits(W5500_NSS_GPIO_PORT, W5500_NSS_PIN);
}

static uint8_t W5500_WriteAndReadSPI(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(W5500_SPI, SPI_I2S_FLAG_TXE) == RESET) { }

    SPI_I2S_SendData(W5500_SPI, data);    

    while (SPI_I2S_GetFlagStatus(W5500_SPI, SPI_I2S_FLAG_RXNE) == RESET) { }
    
    return (uint8_t) SPI_I2S_ReceiveData(W5500_SPI);
}

static void W5500_Write(uint8_t data)
{
    W5500_WriteAndReadSPI(data);
}

static uint8_t W5500_Read()
{
    return W5500_WriteAndReadSPI(0x00);
}

static void W5500_WriteBurst(uint8_t* buffer, uint16_t length)
{
    for (uint16_t index = 0; index < length; index++)
    {
        W5500_WriteAndReadSPI(buffer[index]);
    }    
}

static void W5500_ReadBurst(uint8_t* buffer, uint16_t length)
{
    for (uint16_t index = 0; index < length; index++)
    {
        buffer[index] = W5500_WriteAndReadSPI(0x00);
    }   
}


void W5500_ConfigureSPI(void)
{
    // Clocks
    W5500_SPI_CLK_INIT(W5500_SPI_CLK, ENABLE);

    RCC_AHB1PeriphClockCmd(W5500_SCK_GPIO_CLK | W5500_MISO_GPIO_CLK | 
                           W5500_MOSI_GPIO_CLK | W5500_NSS_GPIO_CLK, ENABLE);
    
    
    // Ports
    GPIO_InitTypeDef port;

    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd  = GPIO_PuPd_DOWN;
    port.GPIO_Speed = GPIO_High_Speed;

    // SCK
    port.GPIO_Pin = W5500_SCK_PIN;
    GPIO_Init(W5500_SCK_GPIO_PORT, &port);
    GPIO_PinAFConfig(W5500_SCK_GPIO_PORT, W5500_SCK_SOURCE, W5500_SCK_AF);

    // MISO
    port.GPIO_Pin =  W5500_MISO_PIN;
    GPIO_Init(W5500_MISO_GPIO_PORT, &port);  
    GPIO_PinAFConfig(W5500_MISO_GPIO_PORT, W5500_MISO_SOURCE, W5500_MISO_AF);    

    // MOSI
    port.GPIO_Pin =  W5500_MOSI_PIN;
    GPIO_Init(W5500_MOSI_GPIO_PORT, &port);
    GPIO_PinAFConfig(W5500_MOSI_GPIO_PORT, W5500_MOSI_SOURCE, W5500_MOSI_AF);
    
    // NSS
    port.GPIO_Pin =  W5500_NSS_PIN;
    port.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(W5500_NSS_GPIO_PORT, &port);
    //GPIO_PinAFConfig(W5500_NSS_GPIO_PORT, W5500_NSS_SOURCE, W5500_NSS_AF);    

    GPIO_SetBits(W5500_NSS_GPIO_PORT, W5500_NSS_PIN);

    
    // SPI
    SPI_I2S_DeInit(W5500_SPI);

    SPI_InitTypeDef spi;
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode = SPI_Mode_Master;
    spi.SPI_DataSize = SPI_DataSize_8b;
    spi.SPI_CPOL = SPI_CPOL_Low;
    spi.SPI_CPHA = SPI_CPHA_1Edge;
    spi.SPI_NSS = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial = 7;
    SPI_Init(W5500_SPI, &spi);
    
    SPI_Cmd(W5500_SPI, ENABLE);
}

uint8_t W5500_Init(void)
{
    reg_wizchip_cs_cbfunc(W5500_ChipSelect, W5500_ChipDeselect);
	reg_wizchip_spi_cbfunc(W5500_Read, W5500_Write);
	reg_wizchip_spiburst_cbfunc(W5500_ReadBurst, W5500_WriteBurst);
    
    uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };
        
    return (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) != -1);
}

void W5500_PrintNetworkSettings(void)
{
	uint8_t tmpstr[6] = {0,};
	wiz_NetInfo gWIZNETINFO;

	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	ctlwizchip(CW_GET_ID,(void*)tmpstr);

	// Display Network Information
	if(gWIZNETINFO.dhcp == NETINFO_DHCP) printf("\r\n===== %s NET CONF : DHCP =====\r\n",(char*)tmpstr);
		else printf("\r\n===== %s NET CONF : Static =====\r\n",(char*)tmpstr);

	printf("\r\nMAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
	printf("IP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
	printf("GW: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
	printf("SN: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
}
