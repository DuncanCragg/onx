
#include "onx/log.h"
#include <onx/spi.h>
#include <onx/gpio.h>

static volatile bool     sending=false;
static volatile uint8_t* curr_data;
static volatile uint16_t curr_len;
static          void     (*spi_done_cb)();

void next_block_of_255() {
    if(!curr_len){
      if(spi_done_cb) spi_done_cb();
      spi_done_cb=0;
 //   PORT gpio_pin_set(SPIM_SS_PIN);
      sending=false;
      return;
    }

    uint8_t m=curr_len>255? 255: curr_len;

//  PORT send curr_data, m

    curr_data+=m;
    curr_len-=m;

#if defined(SPI_BLOCKING)
 // PORT gpio_pin_set(SPIM_SS_PIN);
    sending=false;
#endif
}

/*
void spi_event_handler(..) {
    next_block_of_255();
}
*/

// REVISIT: initialised?
void spi_init() {

// PORT set MOSI MISO CLK CS
// PORT set freq, mode 0, MSB first 

#if defined(SPI_BLOCKING)
//  spi_init(..);
#else
//  spi_init(.. spi_event_handler,);
#endif

}

bool spi_sending() {
  return sending;
}

void spi_tx(uint8_t* data, uint16_t len, void (*cb)()) {

    sending=true;
    curr_data=data;
    curr_len =len;
    spi_done_cb=cb;

//  gpio_pin_clear(SPIM_SS_PIN);
    spi_wake();

    next_block_of_255();

    if(!cb) while(sending);
}

static bool sleeping=false;
void spi_sleep() {
  if(sleeping || sending) return;
  sleeping=true;
}

void spi_wake() {
  if(!sleeping) return;
  sleeping=false;
}

// ------------------------------------------


