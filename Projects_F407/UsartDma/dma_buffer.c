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


#include "dma_buffer.h"

#include <stdio.h>
#include <string.h>


void dmaBufferInit(DmaBuffer* dmaBuffer, uint32_t* dmaCounter, char* buffer,
    size_t size)
{
    dmaBuffer->input = dmaCounter;
    dmaBuffer->output = 0;
    dmaBuffer->size = size;
    dmaBuffer->buffer = buffer;
}

void dmaBufferClear(DmaBuffer* dmaBuffer)
{
    dmaBuffer->output = dmaBuffer->size - *dmaBuffer->input;
}

size_t dmaBufferBytesUsed(DmaBuffer* dmaBuffer)
{
    size_t input = dmaBuffer->size - *dmaBuffer->input;
    
    if (input < dmaBuffer->output)
    {
        return dmaBuffer->size - dmaBuffer->output + input;
    }
    else
    {
        return input - dmaBuffer->output;
    }
}

size_t dmaBufferBytesFree(DmaBuffer* dmaBuffer)
{
    size_t input = dmaBuffer->size - *dmaBuffer->input;

    if (input < dmaBuffer->output)
    {
        return dmaBuffer->output - input - 1;
    }
    else
    {
        return dmaBuffer->size - input + dmaBuffer->output - 1;
    }
}

bool dmaBufferIsEmpty(DmaBuffer* dmaBuffer)
{
    return (dmaBuffer->output == (dmaBuffer->size - *dmaBuffer->input));
}

bool dmaBufferIsFull(DmaBuffer* dmaBuffer)
{
    return (dmaBufferBytesFree(dmaBuffer) == 0);
}

char dmaBufferGetByte(DmaBuffer* dmaBuffer)
{
    if (dmaBufferIsEmpty(dmaBuffer))
        return 0;

    char result = dmaBuffer->buffer[dmaBuffer->output];
    dmaBuffer->output++;

    if (dmaBuffer->output >= dmaBuffer->size)
        dmaBuffer->output = 0;

    return result;
}

size_t dmaBufferGetData(DmaBuffer* dmaBuffer, void* data, size_t size)
{
    size_t result = dmaBufferGetDataInSitu(dmaBuffer, data, size, 0);

    dmaBufferSkip(dmaBuffer, result);

    return result;
}

size_t dmaBufferGetLine(DmaBuffer* dmaBuffer, char* line, size_t size)
{
    size_t result = dmaBufferIndexOfAny(dmaBuffer, "\r\n", 0);

    if (result == DB_INVALID_INDEX)
        return 0;
    
    result++; // Include '\n' character
    
    if (result >= size)
        return 0;

    dmaBufferGetDataInSitu(dmaBuffer, line, result, 0);

    dmaBufferSkip(dmaBuffer, result);

    line[result] = 0;

    return result;
}

char dmaBufferGetByteInSitu(DmaBuffer* dmaBuffer, size_t offset)
{
    if (dmaBufferBytesUsed(dmaBuffer) <= offset)
        return 0;

    if (dmaBuffer->output + offset >= dmaBuffer->size)
    {
        return dmaBuffer->buffer[dmaBuffer->output + offset - 
            dmaBuffer->size];
    }
    else
    {
        return dmaBuffer->buffer[dmaBuffer->output + offset];
    }
}

size_t dmaBufferGetDataInSitu(DmaBuffer* dmaBuffer, void* data, size_t size,
    size_t offset)
{
    size_t limit = dmaBufferBytesUsed(dmaBuffer);

    if (limit <= offset)
        return 0;

    if (size < limit - offset)
        limit = offset + size;

    char value = 0;

    for (size_t i = offset; i < limit; ++i)
    {
        if (dmaBuffer->output + i >= dmaBuffer->size)
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i -
                dmaBuffer->size];
        }
        else
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i];
        }

        ((char*) data)[i - offset] = value;
    }

    return limit - offset;
}

bool dmaBufferStringCompare(DmaBuffer* dmaBuffer, const char* string,
    bool startsWith, size_t limit, size_t offset)
{
    size_t sizeOfString = strlen(string);

    if (startsWith)
    {
        if (limit < sizeOfString)
            return false;

        limit = sizeOfString;
    }
    else if (sizeOfString != limit)
    {
        return false;
    }

    size_t size = dmaBufferBytesUsed(dmaBuffer);

    limit += offset;

    if (size <= limit)
        return false;

    char value = 0;

    for (size_t i = offset; i < limit; ++i)
    {
        if (dmaBuffer->output + i >= dmaBuffer->size)
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i -
                dmaBuffer->size];
        }
        else
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i];
        }

        if (value != string[i - offset])
            return false;
    }

    return true;
}

size_t dmaBufferIndexOf(DmaBuffer* dmaBuffer, char character, size_t offset)
{
    size_t size = dmaBufferBytesUsed(dmaBuffer);

    if (size <= offset)
        return DB_INVALID_INDEX;

    char value = 0;

    for (size_t i = offset; i < size; ++i)
    {
        if (dmaBuffer->output + i >= dmaBuffer->size)
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i -
                dmaBuffer->size];
        }
        else
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i];
        }

        if (value == character)
            return i;
    }

    return DB_INVALID_INDEX;
}

size_t dmaBufferIndexOfAny(DmaBuffer* dmaBuffer, const char* any,
    size_t offset)
{
    size_t size = dmaBufferBytesUsed(dmaBuffer);
    size_t sizeOfAny = strlen(any);

    if (size <= offset || sizeOfAny == 0)
        return DB_INVALID_INDEX;

    char value = 0;

    for (size_t i = offset; i < size; ++i)
    {
        if (dmaBuffer->output + i >= dmaBuffer->size)
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i - 
                dmaBuffer->size];
        }
        else
        {
            value = dmaBuffer->buffer[dmaBuffer->output + i];
        }

        for (size_t j = 0; j < sizeOfAny; ++j)
        {
            if (any[j] == value)
                return i;
        }
    }

    return DB_INVALID_INDEX;
}

void dmaBufferSkip(DmaBuffer* dmaBuffer, size_t count)
{
    size_t input = dmaBuffer->size - *dmaBuffer->input;
    
    if (input < dmaBuffer->output)
    {
        dmaBuffer->output += count;

        if (dmaBuffer->output >= dmaBuffer->size)
        {
            dmaBuffer->output -= dmaBuffer->size;

            if (dmaBuffer->output > input)
                dmaBuffer->output = input;
        }
    }
    else
    {
        dmaBuffer->output += count;

        if (dmaBuffer->output > input)
            dmaBuffer->output = input;
    }
}

void dmaBufferDebug(DmaBuffer* dmaBuffer)
{
    printf("\r\nBuffer:");

    for (size_t i = 0; i < dmaBuffer->size; ++i)
        printf(" %02X", (unsigned char) dmaBuffer->buffer[i]);

    printf("\r\nOffset:");

    for (size_t i = 0; i < dmaBuffer->size; ++i)
        printf(" %02X", (unsigned char) i);

    printf("\r\n Marks:");

    for (size_t i = 0; i < dmaBuffer->size; ++i)
    {
        if (i == dmaBuffer->output)
        {
            printf(" R");
        }
        else
        {
            printf("  ");
        }

        if (i == (dmaBuffer->size - *dmaBuffer->input))
        {
            printf("W");
        }
        else
        {
            printf(" ");
        }
    }

    if (dmaBufferIsEmpty(dmaBuffer))
    {
        printf(" (EMPTY)");
    }
    else if (dmaBufferIsFull(dmaBuffer))
    {
        printf(" (FULL)");
    }

    printf("\r\nUsed: %u, Free: %u, Total: %u\r\n\r\n",
        (unsigned int) dmaBufferBytesUsed(dmaBuffer),
        (unsigned int) dmaBufferBytesFree(dmaBuffer),
        (unsigned int) dmaBuffer->size);
}
