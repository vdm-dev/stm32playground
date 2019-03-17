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


#ifndef DMA_BUFFER_H_INCLUDED
#define DMA_BUFFER_H_INCLUDED


#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


#define DB_INVALID_INDEX ((size_t) -1)


typedef struct
{
    uint32_t* input;
    size_t    output;
    size_t    size;
    char*     buffer;
} DmaBuffer;


void dmaBufferInit(DmaBuffer* dmaBuffer, uint32_t* dmaCounter, char* buffer,
    size_t size);
void dmaBufferClear(DmaBuffer* dmaBuffer);

size_t dmaBufferBytesUsed(DmaBuffer* dmaBuffer);
size_t dmaBufferBytesFree(DmaBuffer* dmaBuffer);

bool dmaBufferIsEmpty(DmaBuffer* dmaBuffer);
bool dmaBufferIsFull(DmaBuffer* dmaBuffer);

char dmaBufferGetByte(DmaBuffer* dmaBuffer);
size_t dmaBufferGetData(DmaBuffer* dmaBuffer, void* data, size_t size);
size_t dmaBufferGetLine(DmaBuffer* dmaBuffer, char* line, size_t size);
char dmaBufferGetByteInSitu(DmaBuffer* dmaBuffer, size_t offset);
size_t dmaBufferGetDataInSitu(DmaBuffer* dmaBuffer, void* data, size_t size,
    size_t offset);

bool dmaBufferStringCompare(DmaBuffer* dmaBuffer, const char* string,
    bool startsWith, size_t limit, size_t offset);

size_t dmaBufferIndexOf(DmaBuffer* dmaBuffer, char character, size_t offset);
size_t dmaBufferIndexOfAny(DmaBuffer* dmaBuffer, const char* any,
    size_t offset);

void dmaBufferSkip(DmaBuffer* dmaBuffer, size_t count);

void dmaBufferDebug(DmaBuffer* dmaBuffer);


#endif // DMA_BUFFER_H_INCLUDED
