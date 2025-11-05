#ifndef DMA_MEM_H
#define DMA_MEM_H

#include <sync-and-mem.h>

// REVISIT platform-independently way to indicate speed critical, i.e. "X", inline, etc

void X dma_memcpy16(void* to, void* from,     size_t len, int chan, bool hi_pri);
void X dma_memset16(void* to, uint16_t value, size_t len, int chan, bool hi_pri);

#endif


