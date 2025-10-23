
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/spi.h"

#include "onx/log.h"
#include <onx/spi.h>
#undef spi_init
#include <onx/gpio.h>

static bool initialised=false;
static bool initialised_2=false;

void spi_init_avoid_sdk() {

  if(initialised) return;

  spi_init(spi_default,  1 * 1000 * 1000);

  gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(PICO_DEFAULT_SPI_TX_PIN,  GPIO_FUNC_SPI);
  gpio_set_function(PICO_DEFAULT_SPI_RX_PIN,  GPIO_FUNC_SPI);

  bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_SCK_PIN,
                             PICO_DEFAULT_SPI_TX_PIN,
                             PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI));
  initialised=true;
}

void spi_init_2(int8_t sck_pin, int8_t tx_pin, int8_t rx_pin, int8_t cs_pin){

  if(initialised_2) return;

  if(sck_pin < 0) return;

  spi_init(spi0, 32 * 1000 * 1000);

  bool do_t=(tx_pin >=0);
  bool do_r=(rx_pin >=0);
  bool do_c=(cs_pin >=0);

  ;        gpio_set_function(sck_pin, GPIO_FUNC_SPI);
  if(do_t) gpio_set_function(tx_pin,  GPIO_FUNC_SPI);
  if(do_r) gpio_set_function(rx_pin,  GPIO_FUNC_SPI);
  if(do_c) gpio_set_function(cs_pin,  GPIO_FUNC_SPI);

  initialised_2=true;

  // REVISIT: what even /is/ bi_decl(), and is it worth all these pretty patterns?
  // REVISIT: it won't compile anyway
/*
  bool t__ =  do_t && !do_r && !do_c;
  bool _r_ = !do_t &&  do_r && !do_c;
  bool tr_ =  do_t &&  do_r && !do_c;
  bool t_c =  do_t && !do_r &&  do_c;
  bool _rc = !do_t &&  do_r &&  do_c;
  bool trc =  do_t &&  do_r &&  do_c;

  if(t__){ bi_decl(bi_2pins_with_func(sck_pin, tx_pin,                 GPIO_FUNC_SPI)); }
  if(_r_){ bi_decl(bi_2pins_with_func(sck_pin,         rx_pin,         GPIO_FUNC_SPI)); }
  if(tr_){ bi_decl(bi_3pins_with_func(sck_pin, tx_pin, rx_pin,         GPIO_FUNC_SPI)); }
  if(t_c){ bi_decl(bi_3pins_with_func(sck_pin, tx_pin,         cs_pin, GPIO_FUNC_SPI)); }
  if(_rc){ bi_decl(bi_3pins_with_func(sck_pin,         rx_pin, cs_pin, GPIO_FUNC_SPI)); }
  if(trc){ bi_decl(bi_4pins_with_func(sck_pin, tx_pin, rx_pin, cs_pin, GPIO_FUNC_SPI)); }
*/
}

uint16_t spi_read(uint8_t* buf, uint16_t len){
  return spi_read_blocking(spi_default, 0, buf, len);
}

void spi_write(uint8_t* buf, uint16_t len) {
  spi_write_blocking(spi_default, buf, len);
}

void spi_write_2(uint8_t* buf, uint16_t len) {
  spi_write_blocking(spi0, buf, len);
}

uint8_t spi_rw_byte(uint8_t out) {
  uint8_t in;
  int n=spi_write_read_blocking(spi_default, &out, &in, 1);
  return in;
}

// ----------------------------------------------------

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


