//
// rfid.c based on MFRC522.cpp - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
// Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
// Created by Miguel Balboa (circuitito.com), Jan, 2012.
// Rewritten by Soren Thing Andersen (access.thing.dk), fall of 2013 (Translation to English, refactored, comments, anti collision, cascade levels.)
// Ported to mbed by Martin Olejar, Dec, 2013
// Ported to C for STM32F407VGT by Dmitry Lavygin (vdm.inbox@gmail.com), Dec, 2017
//


#include "rfid.h"

#include <string.h>

#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_spi.h>


static const char* const _TypeNamePICC[] =
{
  "Unknown type",
  "PICC compliant with ISO/IEC 14443-4",
  "PICC compliant with ISO/IEC 18092 (NFC)",
  "MIFARE Mini, 320 bytes",
  "MIFARE 1KB",
  "MIFARE 4KB",
  "MIFARE Ultralight or Ultralight C",
  "MIFARE Plus",
  "MIFARE TNP3XXX",

  /* not complete UID */
  "SAK indicates UID is not complete"
};

static const char* const _ErrorMessage[] =
{
  "Unknown error",
  "Success",
  "Error in communication",
  "Collision detected",
  "Timeout in communication",
  "A buffer is not big enough",
  "Internal error in the code, should not happen",
  "Invalid argument",
  "The CRC_A does not match",
  "A MIFARE PICC responded with NAK"
};

#define MFRC522_MaxPICCs (sizeof(_TypeNamePICC)/sizeof(_TypeNamePICC[0]))
#define MFRC522_MaxError (sizeof(_ErrorMessage)/sizeof(_ErrorMessage[0]))


void PCD_ConfigureSPI(void)
{
    // Clocks
    RFID_SPI_CLK_INIT(RFID_SPI_CLK, ENABLE);

    RCC_AHB1PeriphClockCmd(RFID_SCK_GPIO_CLK | RFID_MISO_GPIO_CLK | 
                           RFID_MOSI_GPIO_CLK | RFID_NSS_GPIO_CLK, ENABLE);
    
    
    // Ports
    GPIO_InitTypeDef port;

    port.GPIO_Mode = GPIO_Mode_AF;
    port.GPIO_OType = GPIO_OType_PP;
    port.GPIO_PuPd  = GPIO_PuPd_DOWN;
    port.GPIO_Speed = GPIO_High_Speed;

    // SCK
    port.GPIO_Pin = RFID_SCK_PIN;
    GPIO_Init(RFID_SCK_GPIO_PORT, &port);
    GPIO_PinAFConfig(RFID_SCK_GPIO_PORT, RFID_SCK_SOURCE, RFID_SCK_AF);

    // MISO
    port.GPIO_Pin =  RFID_MISO_PIN;
    GPIO_Init(RFID_MISO_GPIO_PORT, &port);  
    GPIO_PinAFConfig(RFID_MISO_GPIO_PORT, RFID_MISO_SOURCE, RFID_MISO_AF);    

    // MOSI
    port.GPIO_Pin =  RFID_MOSI_PIN;
    GPIO_Init(RFID_MOSI_GPIO_PORT, &port);
    GPIO_PinAFConfig(RFID_MOSI_GPIO_PORT, RFID_MOSI_SOURCE, RFID_MOSI_AF);
    
    // NSS
    port.GPIO_Pin =  RFID_NSS_PIN;
    port.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(RFID_NSS_GPIO_PORT, &port);
    //GPIO_PinAFConfig(RFID_NSS_GPIO_PORT, RFID_NSS_SOURCE, RFID_NSS_AF);    

    GPIO_SetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);

    
    // SPI
    SPI_I2S_DeInit(RFID_SPI);

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
    SPI_Init(RFID_SPI, &spi);
    
    SPI_Cmd(RFID_SPI, ENABLE);
}

uint8_t PCD_WriteAndReadSPI(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(RFID_SPI, SPI_I2S_FLAG_TXE) == RESET) { }

    SPI_I2S_SendData(RFID_SPI, data);    

    while (SPI_I2S_GetFlagStatus(RFID_SPI, SPI_I2S_FLAG_RXNE) == RESET) { }
    
    return (uint8_t) SPI_I2S_ReceiveData(RFID_SPI);
}

void PCD_WriteRegister(uint8_t reg, uint8_t value)
{
    GPIO_ResetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
    
    PCD_WriteAndReadSPI(reg & 0x7E);
    PCD_WriteAndReadSPI(value);

    GPIO_SetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
}

void PCD_WriteBuffer(uint8_t reg, uint8_t count, uint8_t *values)
{
    GPIO_ResetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);

    // MSB == 0 is for writing. LSB is not used in address. Datasheet section 8.1.2.3.
    PCD_WriteAndReadSPI(reg & 0x7E);
    
    for (uint8_t index = 0; index < count; index++)
    {
        PCD_WriteAndReadSPI(values[index]);
    }

    GPIO_SetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
}

uint8_t PCD_ReadRegister(uint8_t reg)
{
    GPIO_ResetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
    
    PCD_WriteAndReadSPI(reg | 0x80);
    
    uint8_t result = PCD_WriteAndReadSPI(0);

    GPIO_SetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
    
    return result;
}

void PCD_ReadBuffer(uint8_t reg, uint8_t count, uint8_t *values, uint8_t rxAlign)
{
    if (!count) 
        return;

    uint8_t address = 0x80 | reg;  // MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
    uint8_t index = 0;             // Index in values array.

    GPIO_ResetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
    
    count--;                       // One read is performed outside of the loop
    
    PCD_WriteAndReadSPI(address);  // Tell MFRC522 which address we want to read

    while (index < count)
    {
        if ((index == 0) && rxAlign) // Only update bit positions rxAlign..7 in values[0]
        {
            // Create bit mask for bit positions rxAlign..7
            uint8_t mask = 0;
            
            for (uint8_t i = rxAlign; i <= 7; i++)
            {
                mask |= (1 << i);
            }

            // Read value and tell that we want to read the same address again.
            uint8_t value = PCD_WriteAndReadSPI(address);

            // Apply mask to both current value of values[0] and the new data in value.
            values[0] = (values[index] & ~mask) | (value & mask);
        }
        else
        {
            // Read value and tell that we want to read the same address again.
            values[index] = PCD_WriteAndReadSPI(address);
        }

        index++;
    }

    values[index] = PCD_WriteAndReadSPI(0); // Read the final byte. Send 0 to stop reading.

    GPIO_SetBits(RFID_NSS_GPIO_PORT, RFID_NSS_PIN);
}

void PCD_SetRegisterBits(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = PCD_ReadRegister(reg);
    
    PCD_WriteRegister(reg, tmp | mask);     // set bit mask
}

void PCD_ClrRegisterBits(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = PCD_ReadRegister(reg);
    PCD_WriteRegister(reg, tmp & (~mask));    // clear bit mask
}

uint8_t PCD_CalculateCRC(uint8_t *data, uint8_t length, uint8_t *result)
{
    PCD_WriteRegister(CommandReg, PCD_Idle);      // Stop any active command.
    PCD_WriteRegister(DivIrqReg, 0x04);           // Clear the CRCIRq interrupt request bit
    PCD_SetRegisterBits(FIFOLevelReg, 0x80);      // FlushBuffer = 1, FIFO initialization
    PCD_WriteBuffer(FIFODataReg, length, data);   // Write data to the FIFO
    PCD_WriteRegister(CommandReg, PCD_CalcCRC);   // Start the calculation

    // Wait for the CRC calculation to complete. Each iteration of the while-loop takes 17.73us.
    uint16_t i = 5000;
    uint8_t n;
    
    while (1)
    {
        n = PCD_ReadRegister(DivIrqReg);  // DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq   reserved CRCIRq reserved reserved

        if (n & 0x04)
        {
            // CRCIRq bit set - calculation done
            break;
        }

        if (--i == 0)
        {
            // The emergency break. We will eventually terminate on this one after 89ms.
            // Communication with the MFRC522 might be down.
            return STATUS_TIMEOUT;
        }
    }

    // Stop calculating CRC for new content in the FIFO.
    PCD_WriteRegister(CommandReg, PCD_Idle);

    // Transfer the result from the registers to the result buffer
    result[0] = PCD_ReadRegister(CRCResultRegL);
    result[1] = PCD_ReadRegister(CRCResultRegH);

    return STATUS_OK;
}

void PCD_Init(void)
{
    /* Reset MFRC522 */
    // TODO:
    //m_RESET = 0;
    //wait_ms(10);
    //m_RESET = 1;

    // Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74us. Let us be generous: 50ms.
    //wait_ms(50);

    // When communicating with a PICC we need a timeout if something goes wrong.
    // f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
    // TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
    PCD_WriteRegister(TModeReg, 0x80);      // TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
    PCD_WriteRegister(TPrescalerReg, 0xA9); // TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25us.
    PCD_WriteRegister(TReloadRegH, 0x03);   // Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
    PCD_WriteRegister(TReloadRegL, 0xE8);

    PCD_WriteRegister(TxASKReg, 0x40);      // Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
    PCD_WriteRegister(ModeReg, 0x3D);       // Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)

    PCD_WriteRegister(RFCfgReg, (0x07<<4)); // Set Rx Gain to max

    PCD_AntennaOn();                        // Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
}

void PCD_Reset(void)
{
    PCD_WriteRegister(CommandReg, PCD_SoftReset); // Issue the SoftReset command.
    // The datasheet does not mention how long the SoftRest command takes to complete.
    // But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg)
    // Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74us. Let us be generous: 50ms.
    //wait_ms(50);

    // Wait for the PowerDown bit in CommandReg to be cleared
    while (PCD_ReadRegister(CommandReg) & (1<<4))
    {
        // PCD still restarting - unlikely after waiting 50ms, but better safe than sorry.
    }
}

void PCD_AntennaOn(void)
{
    uint8_t value = PCD_ReadRegister(TxControlReg);
    
    if ((value & 0x03) != 0x03)
        PCD_WriteRegister(TxControlReg, value | 0x03);
}

uint8_t PCD_TransceiveData(uint8_t *sendData,
                           uint8_t sendLen,
                           uint8_t *backData,
                           uint8_t *backLen,
                           uint8_t *validBits,
                           uint8_t rxAlign,
                           uint8_t checkCRC)
{
    uint8_t waitIRq = 0x30;    // RxIRq and IdleIRq

    return PCD_CommunicateWithPICC(PCD_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
}

uint8_t PCD_CommunicateWithPICC(uint8_t command,
                                uint8_t waitIRq,
                                uint8_t *sendData,
                                uint8_t sendLen,
                                uint8_t *backData,
                                uint8_t *backLen,
                                uint8_t *validBits,
                                uint8_t rxAlign,
                                uint8_t checkCRC)
{
    uint8_t n, _validBits = 0;
    uint32_t i;

    // Prepare values for BitFramingReg
    uint8_t txLastBits = validBits ? *validBits : 0;
    uint8_t bitFraming = (rxAlign << 4) + txLastBits;   // RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]

    PCD_WriteRegister(CommandReg, PCD_Idle);            // Stop any active command.
    PCD_WriteRegister(ComIrqReg, 0x7F);                 // Clear all seven interrupt request bits
    PCD_SetRegisterBits(FIFOLevelReg, 0x80);            // FlushBuffer = 1, FIFO initialization
    PCD_WriteBuffer(FIFODataReg, sendLen, sendData);    // Write sendData to the FIFO
    PCD_WriteRegister(BitFramingReg, bitFraming);       // Bit adjustments
    PCD_WriteRegister(CommandReg, command);             // Execute the command

    if (command == PCD_Transceive)
        PCD_SetRegisterBits(BitFramingReg, 0x80);       // StartSend=1, transmission of data starts

    // Wait for the command to complete.
    // In PCD_Init() we set the TAuto flag in TModeReg. This means the timer automatically starts when the PCD stops transmitting.
    // Each iteration of the do-while-loop takes 17.86us.
    i = 2000;
    while (1)
    {
        n = PCD_ReadRegister(ComIrqReg);  // ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq   HiAlertIRq LoAlertIRq ErrIRq TimerIRq

        if (n & waitIRq)
        {          // One of the interrupts that signal success has been set.
            break;
        }

        if (n & 0x01)
        {           // Timer interrupt - nothing received in 25ms
            return STATUS_TIMEOUT;
        }

        if (--i == 0)
        {           // The emergency break. If all other condions fail we will eventually terminate on this one after 35.7ms. Communication with the MFRC522 might be down.
            return STATUS_TIMEOUT;
        }
    }

    // Stop now if any errors except collisions were detected.
    uint8_t errorRegValue = PCD_ReadRegister(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl   CollErr CRCErr ParityErr ProtocolErr
    
    if (errorRegValue & 0x13)
    {  // BufferOvfl ParityErr ProtocolErr
        return STATUS_ERROR;
    }

    // If the caller wants data back, get it from the MFRC522.
    if (backData && backLen)
    {
        n = PCD_ReadRegister(FIFOLevelReg);           // Number of bytes in the FIFO
        
        if (n > *backLen)
        {
            return STATUS_NO_ROOM;
        }

        *backLen = n;                       // Number of bytes returned
        
        PCD_ReadBuffer(FIFODataReg, n, backData, rxAlign);    // Get received data from FIFO
        
        _validBits = PCD_ReadRegister(ControlReg) & 0x07; // RxLastBits[2:0] indicates the number of valid bits in the last received byte. If this value is 000b, the whole byte is valid.
        
        if (validBits)
        {
            *validBits = _validBits;
        }
    }

    // Tell about collisions
    if (errorRegValue & 0x08)
    { // CollErr
        return STATUS_COLLISION;
    }

    // Perform CRC_A validation if requested.
    if (backData && backLen && checkCRC)
    {
        // In this case a MIFARE Classic NAK is not OK.
        if ((*backLen == 1) && (_validBits == 4))
        {
            return STATUS_MIFARE_NACK;
        }

        // We need at least the CRC_A value and all 8 bits of the last byte must be received.
        if ((*backLen < 2) || (_validBits != 0))
        {
            return STATUS_CRC_WRONG;
        }

        // Verify CRC_A - do our own calculation and store the control in controlBuffer.
        uint8_t controlBuffer[2];
        
        n = PCD_CalculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
        
        if (n != STATUS_OK)
        {
            return n;
        }

        if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1]))
        {
            return STATUS_CRC_WRONG;
        }
    }

    return STATUS_OK;
}

uint8_t PICC_REQA_or_WUPA(uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize)
{
    uint8_t validBits;
    uint8_t status;

    if (bufferATQA == 0 || *bufferSize < 2)
    {  // The ATQA response is 2 bytes long.
        return STATUS_NO_ROOM;
    }

    // ValuesAfterColl=1 => Bits received after collision are cleared.
    PCD_ClrRegisterBits(CollReg, 0x80);

    // For REQA and WUPA we need the short frame format
    // - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
    validBits = 7;

    status = PCD_TransceiveData(&command, 1, bufferATQA, bufferSize, &validBits, 0, 0);
    
    if (status != STATUS_OK)
    {
        return status;
    }

    if ((*bufferSize != 2) || (validBits != 0))
    {   // ATQA must be exactly 16 bits.
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

uint8_t PICC_RequestA(uint8_t *bufferATQA, uint8_t *bufferSize)
{
    return PICC_REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, bufferSize);
}

uint8_t PICC_WakeupA(uint8_t *bufferATQA, uint8_t *bufferSize)
{
  return PICC_REQA_or_WUPA(PICC_CMD_WUPA, bufferATQA, bufferSize);
}

uint8_t PICC_Select(Uid *uid, uint8_t validBits)
{
    uint8_t uidComplete;
    uint8_t selectDone;
    uint8_t useCascadeTag;
    uint8_t cascadeLevel = 1;
    uint8_t result;
    uint8_t count;
    uint8_t index;
    uint8_t uidIndex;          // The first index in uid->uidByte[] that is used in the current Cascade Level.
    uint8_t currentLevelKnownBits;   // The number of known UID bits in the current Cascade Level.
    uint8_t buffer[9];         // The SELECT/ANTICOLLISION commands uses a 7 byte standard frame + 2 bytes CRC_A
    uint8_t bufferUsed;        // The number of bytes used in the buffer, ie the number of bytes to transfer to the FIFO.
    uint8_t rxAlign;           // Used in BitFramingReg. Defines the bit position for the first bit received.
    uint8_t txLastBits;        // Used in BitFramingReg. The number of valid bits in the last transmitted byte.
    uint8_t *responseBuffer;
    uint8_t responseLength;

    // Description of buffer structure:
    //    Byte 0: SEL         Indicates the Cascade Level: PICC_CMD_SEL_CL1, PICC_CMD_SEL_CL2 or PICC_CMD_SEL_CL3
    //    Byte 1: NVB         Number of Valid Bits (in complete command, not just the UID): High nibble: complete bytes, Low nibble: Extra bits.
    //    Byte 2: UID-data or CT    See explanation below. CT means Cascade Tag.
    //    Byte 3: UID-data
    //    Byte 4: UID-data
    //    Byte 5: UID-data
    //    Byte 6: BCC         Block Check Character - XOR of bytes 2-5
    //    Byte 7: CRC_A
    //    Byte 8: CRC_A
    // The BCC and CRC_A is only transmitted if we know all the UID bits of the current Cascade Level.
    //
    // Description of bytes 2-5: (Section 6.5.4 of the ISO/IEC 14443-3 draft: UID contents and cascade levels)
    //    UID size  Cascade level Byte2 Byte3 Byte4 Byte5
    //    ========  ============= ===== ===== ===== =====
    //     4 bytes    1     uid0  uid1  uid2  uid3
    //     7 bytes    1     CT    uid0  uid1  uid2
    //                2     uid3  uid4  uid5  uid6
    //    10 bytes    1     CT    uid0  uid1  uid2
    //                2     CT    uid3  uid4  uid5
    //                3     uid6  uid7  uid8  uid9

    // Sanity checks
    if (validBits > 80)
    {
        return STATUS_INVALID;
    }

    // Prepare MFRC522
    // ValuesAfterColl=1 => Bits received after collision are cleared.
    PCD_ClrRegisterBits(CollReg, 0x80);

    // Repeat Cascade Level loop until we have a complete UID.
    uidComplete = 0;
    while (!uidComplete)
    {
        // Set the Cascade Level in the SEL byte, find out if we need to use the Cascade Tag in byte 2.
        switch (cascadeLevel)
        {
        case 1:
            buffer[0] = PICC_CMD_SEL_CL1;
            uidIndex = 0;
            useCascadeTag = validBits && (uid->size > 4); // When we know that the UID has more than 4 bytes
            break;

        case 2:
            buffer[0] = PICC_CMD_SEL_CL2;
            uidIndex = 3;
            useCascadeTag = validBits && (uid->size > 7); // When we know that the UID has more than 7 bytes
            break;

        case 3:
            buffer[0] = PICC_CMD_SEL_CL3;
            uidIndex = 6;
            useCascadeTag = 0;            // Never used in CL3.
            break;

        default:
            return STATUS_INTERNAL_ERROR;
            //break;
        }

        // How many UID bits are known in this Cascade Level?
        if(validBits > (8 * uidIndex))
        {
            currentLevelKnownBits = validBits - (8 * uidIndex);
        }
        else
        {
            currentLevelKnownBits = 0;
        }

        // Copy the known bits from uid->uidByte[] to buffer[]
        index = 2; // destination index in buffer[]
        if (useCascadeTag)
        {
            buffer[index++] = PICC_CMD_CT;
        }

        uint8_t bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of bytes needed to represent the known bits for this level.
        if (bytesToCopy)
        {
            // Max 4 bytes in each Cascade Level. Only 3 left if we use the Cascade Tag
            uint8_t maxBytes = useCascadeTag ? 3 : 4;
            if (bytesToCopy > maxBytes)
            {
                bytesToCopy = maxBytes;
            }

            for (count = 0; count < bytesToCopy; count++)
            {
                buffer[index++] = uid->uidByte[uidIndex + count];
            }
        }

        // Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
        if (useCascadeTag)
        {
            currentLevelKnownBits += 8;
        }

        // Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
        selectDone = 0;
        while (!selectDone)
        {
            // Find out how many bits and bytes to send and receive.
            if (currentLevelKnownBits >= 32)
            { // All UID bits in this Cascade Level are known. This is a SELECT.
                //Serial.print("SELECT: currentLevelKnownBits="); Serial.println(currentLevelKnownBits, DEC);
                buffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole bytes

                // Calulate BCC - Block Check Character
                buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];

                // Calculate CRC_A
                result = PCD_CalculateCRC(buffer, 7, &buffer[7]);
                if (result != STATUS_OK)
                {
                    return result;
                }

                txLastBits      = 0; // 0 => All 8 bits are valid.
                bufferUsed      = 9;

                // Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
                responseBuffer  = &buffer[6];
                responseLength  = 3;
            }
            else
            { // This is an ANTICOLLISION.
                //Serial.print("ANTICOLLISION: currentLevelKnownBits="); Serial.println(currentLevelKnownBits, DEC);
                txLastBits     = currentLevelKnownBits % 8;
                count          = currentLevelKnownBits / 8;  // Number of whole bytes in the UID part.
                index          = 2 + count;                  // Number of whole bytes: SEL + NVB + UIDs
                buffer[1]      = (index << 4) + txLastBits;  // NVB - Number of Valid Bits
                bufferUsed     = index + (txLastBits ? 1 : 0);

                // Store response in the unused part of buffer
                responseBuffer = &buffer[index];
                responseLength = sizeof(buffer) - index;
            }

            // Set bit adjustments
            rxAlign = txLastBits;                     // Having a seperate variable is overkill. But it makes the next line easier to read.
            PCD_WriteRegister(BitFramingReg, (rxAlign << 4) + txLastBits);  // RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]

            // Transmit the buffer and receive the response.
            result = PCD_TransceiveData(buffer, bufferUsed, responseBuffer, &responseLength, &txLastBits, rxAlign, 0);
            if (result == STATUS_COLLISION)
            { // More than one PICC in the field => collision.
                result = PCD_ReadRegister(CollReg);     // CollReg[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
                if (result & 0x20)
                { // CollPosNotValid
                    return STATUS_COLLISION; // Without a valid collision position we cannot continue
                }

                uint8_t collisionPos = result & 0x1F; // Values 0-31, 0 means bit 32.
                if (collisionPos == 0)
                {
                    collisionPos = 32;
                }

                if (collisionPos <= currentLevelKnownBits)
                { // No progress - should not happen
                    return STATUS_INTERNAL_ERROR;
                }

                // Choose the PICC with the bit set.
                currentLevelKnownBits = collisionPos;
                count          = (currentLevelKnownBits - 1) % 8; // The bit to modify
                index          = 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First byte is index 0.
                buffer[index] |= (1 << count);
            }
            else if (result != STATUS_OK)
            {
                return result;
            }
            else
            { // STATUS_OK
                if (currentLevelKnownBits >= 32)
                { // This was a SELECT.
                    selectDone = 1; // No more anticollision
                    // We continue below outside the while.
                }
                else
                { // This was an ANTICOLLISION.
                    // We now have all 32 bits of the UID in this Cascade Level
                    currentLevelKnownBits = 32;
                    // Run loop again to do the SELECT.
                }
            }
        } // End of while ( ! selectDone)

        // We do not check the CBB - it was constructed by us above.

        // Copy the found UID bytes from buffer[] to uid->uidByte[]
        index       = (buffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
        bytesToCopy = (buffer[2] == PICC_CMD_CT) ? 3 : 4;
        for (count = 0; count < bytesToCopy; count++)
        {
            uid->uidByte[uidIndex + count] = buffer[index++];
        }

        // Check response SAK (Select Acknowledge)
        if (responseLength != 3 || txLastBits != 0)
        {   // SAK must be exactly 24 bits (1 byte + CRC_A).
            return STATUS_ERROR;
        }

        // Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those bytes are not needed anymore.
        result = PCD_CalculateCRC(responseBuffer, 1, &buffer[2]);
        if (result != STATUS_OK)
        {
            return result;
        }

        if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2]))
        {
            return STATUS_CRC_WRONG;
        }

        if (responseBuffer[0] & 0x04)
        { // Cascade bit set - UID not complete yes
            cascadeLevel++;
        }
        else
        {
            uidComplete = 1;
            uid->sak = responseBuffer[0];
        }
    } // End of while ( ! uidComplete)

    // Set correct uid->size
    uid->size = 3 * cascadeLevel + 1;

    return STATUS_OK;
}

uint8_t PICC_HaltA(void)
{
    uint8_t result;
    uint8_t buffer[4];

    // Build command buffer
    buffer[0] = PICC_CMD_HLTA;
    buffer[1] = 0;

    // Calculate CRC_A
    result = PCD_CalculateCRC(buffer, 2, &buffer[2]);
    if (result == STATUS_OK)
    {
        // Send the command.
        // The standard says:
        //    If the PICC responds with any modulation during a period of 1 ms after the end of the frame containing the
        //    HLTA command, this response shall be interpreted as 'not acknowledge'.
        // We interpret that this way: Only STATUS_TIMEOUT is an success.
        result = PCD_TransceiveData(buffer, sizeof(buffer), 0, 0, 0, 0, 0);
        if (result == STATUS_TIMEOUT)
        {
            result = STATUS_OK;
        }
        else if (result == STATUS_OK)
        { // That is ironically NOT ok in this case ;-)
            result = STATUS_ERROR;
        }
    }

    return result;
}

uint8_t PCD_Authenticate(uint8_t command, uint8_t blockAddr, MIFARE_Key *key, Uid *uid)
{
    uint8_t i, waitIRq = 0x10;    // IdleIRq

    // Build command buffer
    uint8_t sendData[12];
    sendData[0] = command;
    sendData[1] = blockAddr;

    for (i = 0; i < MF_KEY_SIZE; i++)
    {  // 6 key bytes
        sendData[2+i] = key->keyByte[i];
    }

    for (i = 0; i < 4; i++)
    { // The first 4 bytes of the UID
        sendData[8+i] = uid->uidByte[i];
    }

    // Start the authentication.
    return PCD_CommunicateWithPICC(PCD_MFAuthent, waitIRq, &sendData[0], sizeof(sendData), 0, 0, 0, 0, 0);
}

void PCD_StopCrypto1(void)
{
    // Clear MFCrypto1On bit
    PCD_ClrRegisterBits(Status2Reg, 0x08); // Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved   MFCrypto1On ModemState[2:0]
}

uint8_t MIFARE_Read(uint8_t blockAddr, uint8_t *buffer, uint8_t *bufferSize)
{
    uint8_t result = STATUS_NO_ROOM;

    // Sanity check
    if ((buffer == 0) || (*bufferSize < 18))
    {
        return result;
    }

    // Build command buffer
    buffer[0] = PICC_CMD_MF_READ;
    buffer[1] = blockAddr;

    // Calculate CRC_A
    result = PCD_CalculateCRC(buffer, 2, &buffer[2]);
    if (result != STATUS_OK)
    {
        return result;
    }

    // Transmit the buffer and receive the response, validate CRC_A.
    return PCD_TransceiveData(buffer, 4, buffer, bufferSize, 0, 0, 1);
}

uint8_t MIFARE_Write(uint8_t blockAddr, uint8_t *buffer, uint8_t bufferSize)
{
    uint8_t result;

    // Sanity check
    if (buffer == 0 || bufferSize < 16)
    {
        return STATUS_INVALID;
    }

    // Mifare Classic protocol requires two communications to perform a write.
    // Step 1: Tell the PICC we want to write to block blockAddr.
    uint8_t cmdBuffer[2];
    cmdBuffer[0] = PICC_CMD_MF_WRITE;
    cmdBuffer[1] = blockAddr;
    // Adds CRC_A and checks that the response is MF_ACK.
    result = PCD_MIFARE_Transceive(cmdBuffer, 2, 0);
    if (result != STATUS_OK)
    {
        return result;
    }

    // Step 2: Transfer the data
    // Adds CRC_A and checks that the response is MF_ACK.
    result = PCD_MIFARE_Transceive(buffer, bufferSize, 0);
    if (result != STATUS_OK)
    {
        return result;
    }

    return STATUS_OK;
}

uint8_t MIFARE_UltralightWrite(uint8_t page, uint8_t *buffer, uint8_t bufferSize)
{
    uint8_t result;

    // Sanity check
    if (buffer == NULL || bufferSize < 4)
    {
        return STATUS_INVALID;
    }

    // Build commmand buffer
    uint8_t cmdBuffer[6];
    cmdBuffer[0] = PICC_CMD_UL_WRITE;
    cmdBuffer[1] = page;
    memcpy(&cmdBuffer[2], buffer, 4);

    // Perform the write
    result = PCD_MIFARE_Transceive(cmdBuffer, 6, 0); // Adds CRC_A and checks that the response is MF_ACK.
    if (result != STATUS_OK)
    {
        return result;
    }

    return STATUS_OK;
}

uint8_t MIFARE_TwoStepHelper(uint8_t command, uint8_t blockAddr, uint32_t data)
{
    uint8_t result;
    uint8_t cmdBuffer[2]; // We only need room for 2 bytes.

    // Step 1: Tell the PICC the command and block address
    cmdBuffer[0] = command;
    cmdBuffer[1] = blockAddr;

    // Adds CRC_A and checks that the response is MF_ACK.
    result = PCD_MIFARE_Transceive(cmdBuffer, 2, 0);
    if (result != STATUS_OK)
    {
    return result;
    }

    // Step 2: Transfer the data
    // Adds CRC_A and accept timeout as success.
    result = PCD_MIFARE_Transceive((uint8_t *) &data, 4, 1);
    if (result != STATUS_OK)
    {
    return result;
    }

    return STATUS_OK;
}

uint8_t MIFARE_Decrement(uint8_t blockAddr, uint32_t delta)
{
    return MIFARE_TwoStepHelper(PICC_CMD_MF_DECREMENT, blockAddr, delta);
}

uint8_t MIFARE_Increment(uint8_t blockAddr, uint32_t delta)
{
    return MIFARE_TwoStepHelper(PICC_CMD_MF_INCREMENT, blockAddr, delta);
}

uint8_t MIFARE_Restore(uint8_t blockAddr)
{
    // The datasheet describes Restore as a two step operation, but does not explain what data to transfer in step 2.
    // Doing only a single step does not work, so I chose to transfer 0L in step two.
    return MIFARE_TwoStepHelper(PICC_CMD_MF_RESTORE, blockAddr, 0L);
}

uint8_t MIFARE_Transfer(uint8_t blockAddr)
{
    uint8_t cmdBuffer[2]; // We only need room for 2 bytes.

    // Tell the PICC we want to transfer the result into block blockAddr.
    cmdBuffer[0] = PICC_CMD_MF_TRANSFER;
    cmdBuffer[1] = blockAddr;

    // Adds CRC_A and checks that the response is MF_ACK.
    return PCD_MIFARE_Transceive(cmdBuffer, 2, 0);
}

uint8_t PCD_MIFARE_Transceive(uint8_t *sendData, uint8_t sendLen, uint8_t acceptTimeout)
{
    uint8_t result;
    uint8_t cmdBuffer[18]; // We need room for 16 bytes data and 2 bytes CRC_A.

    // Sanity check
    if (sendData == NULL || sendLen > 16)
    {
        return STATUS_INVALID;
    }

    // Copy sendData[] to cmdBuffer[] and add CRC_A
    memcpy(cmdBuffer, sendData, sendLen);
    result = PCD_CalculateCRC(cmdBuffer, sendLen, &cmdBuffer[sendLen]);
    if (result != STATUS_OK)
    {
        return result;
    }

    sendLen += 2;

    // Transceive the data, store the reply in cmdBuffer[]
    uint8_t waitIRq = 0x30;    // RxIRq and IdleIRq
    uint8_t cmdBufferSize = sizeof(cmdBuffer);
    uint8_t validBits = 0;
    result = PCD_CommunicateWithPICC(PCD_Transceive, waitIRq, cmdBuffer, sendLen, cmdBuffer, &cmdBufferSize, &validBits, 0, 0);
    if (acceptTimeout && result == STATUS_TIMEOUT)
    {
        return STATUS_OK;
    }

    if (result != STATUS_OK)
    {
        return result;
    }

    // The PICC must reply with a 4 bit ACK
    if (cmdBufferSize != 1 || validBits != 4)
    {
        return STATUS_ERROR;
    }

    if (cmdBuffer[0] != MF_ACK)
    {
        return STATUS_MIFARE_NACK;
    }

    return STATUS_OK;
}

uint8_t PICC_GetType(uint8_t sak)
{
    uint8_t retType = PICC_TYPE_UNKNOWN;

    if (sak & 0x04)
    { // UID not complete
        retType = PICC_TYPE_NOT_COMPLETE;
    }
    else
    {
        switch (sak)
        {
        case 0x09: retType = PICC_TYPE_MIFARE_MINI; break;
        case 0x08: retType = PICC_TYPE_MIFARE_1K;   break;
        case 0x18: retType = PICC_TYPE_MIFARE_4K;   break;
        case 0x00: retType = PICC_TYPE_MIFARE_UL;   break;
        case 0x10:
        case 0x11: retType = PICC_TYPE_MIFARE_PLUS; break;
        case 0x01: retType = PICC_TYPE_TNP3XXX;     break;
        default:
            if (sak & 0x20)
            {
                retType = PICC_TYPE_ISO_14443_4;
            }
            else if (sak & 0x40)
            {
                retType = PICC_TYPE_ISO_18092;
            }
            break;
        }
    }

    return retType;
}

char* PICC_GetTypeName(uint8_t piccType)
{
    if(piccType == PICC_TYPE_NOT_COMPLETE)
    {
        piccType = MFRC522_MaxPICCs - 1;
    }

    return((char *) _TypeNamePICC[piccType]);
}

char* GetStatusCodeName(uint8_t code)
{
    return ((char *) _ErrorMessage[code]);
}

void MIFARE_SetAccessBits(uint8_t *accessBitBuffer,  
                          uint8_t g0,                
                          uint8_t g1,                
                          uint8_t g2,                
                          uint8_t g3)
{
    uint8_t c1 = ((g3 & 4) << 1) | ((g2 & 4) << 0) | ((g1 & 4) >> 1) | ((g0 & 4) >> 2);
    uint8_t c2 = ((g3 & 2) << 2) | ((g2 & 2) << 1) | ((g1 & 2) << 0) | ((g0 & 2) >> 1);
    uint8_t c3 = ((g3 & 1) << 3) | ((g2 & 1) << 2) | ((g1 & 1) << 1) | ((g0 & 1) << 0);

    accessBitBuffer[0] = (~c2 & 0xF) << 4 | (~c1 & 0xF);
    accessBitBuffer[1] =          c1 << 4 | (~c3 & 0xF);
    accessBitBuffer[2] =          c3 << 4 | c2;
}

uint8_t PICC_IsNewCardPresent(void)
{
    uint8_t bufferATQA[2];
    uint8_t bufferSize = sizeof(bufferATQA);
    uint8_t result = PICC_RequestA(bufferATQA, &bufferSize);
    return ((result == STATUS_OK) || (result == STATUS_COLLISION));
}
