
#include <boards.h>

#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/spi.h>

// REVISIT: initialised?

void spi_init(int8_t sck_pin, int8_t mosi_pin, int8_t miso_pin, int8_t cs_pin) {

  gpio_mode(sck_pin,  GPIO_MODE_OUTPUT);
  gpio_mode(mosi_pin, GPIO_MODE_OUTPUT);
  gpio_mode(cs_pin,   GPIO_MODE_OUTPUT);

  gpio_set(sck_pin, 1);
  gpio_set(mosi_pin, 1);
  gpio_set(cs_pin, 1);

  NRF_SPIM3->PSELSCK  = sck_pin;
  NRF_SPIM3->PSELMOSI = mosi_pin;
  NRF_SPIM3->PSELMISO = miso_pin;
  NRF_SPIM3->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M32;
  NRF_SPIM3->INTENSET = 0;
  NRF_SPIM3->ORC = 255;
  NRF_SPIM3->CONFIG = 0;  // MSB first; Mode 0 - see sdk/modules/nrfx/hal/nrf_spim.h
}

static void enable_workaround(NRF_SPIM_Type * spim, uint32_t ppi_channel, uint32_t gpiote_channel) {
  NRF_GPIOTE->CONFIG[gpiote_channel] = (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
                                       (spim->PSEL.SCK << GPIOTE_CONFIG_PSEL_Pos) |
                                       (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);

  NRF_PPI->CH[ppi_channel].EEP = (uint32_t) &NRF_GPIOTE->EVENTS_IN[gpiote_channel];
  NRF_PPI->CH[ppi_channel].TEP = (uint32_t) &spim->TASKS_STOP;
  NRF_PPI->CHENSET = 1U << ppi_channel;
}

static void disable_workaround(NRF_SPIM_Type * spim, uint32_t ppi_channel, uint32_t gpiote_channel) {
  NRF_GPIOTE->CONFIG[gpiote_channel] = 0;
  NRF_PPI->CH[ppi_channel].EEP = 0;
  NRF_PPI->CH[ppi_channel].TEP = 0;
  NRF_PPI->CHENSET = ppi_channel;
}

void spi_read(uint8_t* buf, uint32_t len) {

  if(len == 1) enable_workaround(NRF_SPIM3, 8, 8);
  else         disable_workaround(NRF_SPIM3, 8, 8);

  uint32_t offset = 0;
  do {
    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->EVENTS_ENDTX = 0;

    NRF_SPIM3->TXD.PTR = 0;
    NRF_SPIM3->TXD.MAXCNT = 0;

    NRF_SPIM3->RXD.PTR = (uint32_t)buf + offset;
    if(len <= 0xFF){
      NRF_SPIM3->RXD.MAXCNT = len;
      offset += len;
      len = 0;
    } else {
      NRF_SPIM3->RXD.MAXCNT = 255;
      offset += 255;
      len -= 255;
    }
    NRF_SPIM3->TASKS_START = 1;
    while (NRF_SPIM3->EVENTS_END == 0);
    NRF_SPIM3->EVENTS_END = 0;

  } while(len);
}

void spi_write(uint8_t* buf, uint32_t len) {

  if(len == 1) enable_workaround(NRF_SPIM3, 8, 8);
  else         disable_workaround(NRF_SPIM3, 8, 8);

  uint32_t offset = 0;
  do {
    NRF_SPIM3->EVENTS_END = 0;
    NRF_SPIM3->EVENTS_ENDRX = 0;
    NRF_SPIM3->EVENTS_ENDTX = 0;

    NRF_SPIM3->TXD.PTR = (uint32_t)buf + offset;
    if(len <= 0xFF){
      NRF_SPIM3->TXD.MAXCNT = len;
      offset += len;
      len = 0;
    } else {
      NRF_SPIM3->TXD.MAXCNT = 255;
      offset += 255;
      len -= 255;
    }
    NRF_SPIM3->RXD.PTR = 0;
    NRF_SPIM3->RXD.MAXCNT = 0;

    NRF_SPIM3->TASKS_START = 1;
    while (NRF_SPIM3->EVENTS_END == 0);
    NRF_SPIM3->EVENTS_END = 0;

  } while(len);
}

// -------------------------------------

void spi_enable(bool state) {
  if (state) NRF_SPIM3->ENABLE = 7;
  else       NRF_SPIM3->ENABLE = 0;
}

// -------------------------------------

// for RFM69 if I do that
uint8_t spi_rw_byte(uint8_t out){ return 0; }

// -------------------------------------

static bool sleeping=false;

void spi_sleep() {
  if(sleeping) return;
  sleeping=true;
  NRF_SPIM3->ENABLE=(SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos);
}

void spi_wake() {
  if(!sleeping) return;
  sleeping=false;
  NRF_SPIM3->ENABLE=(SPIM_ENABLE_ENABLE_Enabled  << SPIM_ENABLE_ENABLE_Pos);
}

// -------------------------------------


