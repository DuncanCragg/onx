
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/time.h>

#include <hardware/dma.h>

#include <onx/dma-mem.h>

void __not_in_flash_func(dma_memcpy16)(void* to, void* from, size_t len, int chan, bool hi_pri){

  dma_channel_config c = dma_channel_get_default_config(chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, true);
  channel_config_set_write_increment(&c, true);
  channel_config_set_high_priority(&c, hi_pri);

  dma_channel_configure(
      chan,
      &c,
      to,
      from,
      len,
      true
  );
  dma_channel_wait_for_finish_blocking(chan);
}

void __not_in_flash_func(dma_memset16)(void* to, uint16_t value, size_t len, int chan, bool hi_pri) {

  dma_channel_config c = dma_channel_get_default_config(chan);
  channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_high_priority(&c, hi_pri);

  dma_channel_configure(
      chan,
      &c,
      to,
      &value,
      len,
      true
  );
  dma_channel_wait_for_finish_blocking(chan);
}


