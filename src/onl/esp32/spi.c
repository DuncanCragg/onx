
#include <stdio.h>
#include <string.h>

#include <onx/log.h>
#include <onx/spi.h>
#undef spi_init
#include <onx/gpio.h>

static bool initialised=false;
static bool initialised_2=false;

void spi_init_avoid_sdk() {

  if(initialised) return;

  initialised=true;
}

void spi_init_2(int8_t sck_pin, int8_t tx_pin, int8_t rx_pin, int8_t cs_pin){

  if(initialised_2) return;

  initialised_2=true;
}

uint16_t spi_read(uint8_t* buf, uint16_t len){
  return 0;
}

void spi_write(uint8_t* buf, uint16_t len) {
  0;
}

void spi_write_2(uint8_t* buf, uint16_t len) {
  0;
}

uint8_t spi_rw_byte(uint8_t out) {
  uint8_t in;
  int n=0;
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


