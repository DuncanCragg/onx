
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/spi.h"

#include "onx/log.h"
#include <onx/spi.h>
#undef spi_init
#include <onx/gpio.h>

// REVISIT: initialised?
void spi_init_avoid_sdk() {
#ifdef PICO_DEFAULT_SPI1_SCK_PIN
  spi_init(spi1, 32 * 1000 * 1000);

  gpio_set_function(PICO_DEFAULT_SPI1_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(PICO_DEFAULT_SPI1_TX_PIN,  GPIO_FUNC_SPI);

  bi_decl(bi_2pins_with_func(PICO_DEFAULT_SPI1_TX_PIN, PICO_DEFAULT_SPI1_SCK_PIN, GPIO_FUNC_SPI));
#endif
}

void spi_write(uint8_t* data, uint16_t len) {
  spi_write_blocking(spi1, data, len);
}

static bool sleeping=false;

void spi_sleep() {
  if(sleeping) return;
  sleeping=true;
}

void spi_wake() {
  if(!sleeping) return;
  sleeping=false;
}

// ------------------------------------------


