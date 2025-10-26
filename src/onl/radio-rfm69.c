
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pico/stdlib.h>
#include <pico-support.h>

#include <onx/time.h>
#include <onx/log.h>
#include <onx/mem.h>
#include <onx/gpio.h>
#include <onx/spi.h>
#include <onx/radio.h>
#include <onx/chunkbuf.h>
#include <onx/show_bytes_n_chars.h>

#define NO_TRACK_CHUNK_ASSEMBLY

// --------------------------------------------------------------

#define RFM69_MAX_MESSAGE_LEN 64

#define RADIO_READ_BUFFER_SIZE  2048
#define RADIO_WRITE_BUFFER_SIZE 2048

// --------------------------------------------------------------
// REVISIT: when do I need chunkbuf_clear(radio_write_buf);

static volatile bool initialised=false;

static volatile channel_recv_cb recv_cb = 0;

static volatile chunkbuf* radio_read_buf = 0;
static volatile chunkbuf* radio_write_buf = 0;

// ------ declare functions from RadioHead C port --------------

static bool rh_init();
static void rh_write_fifo(uint8_t* buf, size_t len);
static void rh_set_mode_rx();
static void rh_set_mode_tx();
static void rh_set_mode_standby();
static void rh_sleep();

// -------------------------------------------------------------

extern const list* onp_radio_bands;

bool radio_init(channel_recv_cb cb){

  recv_cb = cb;

  if(initialised) return true;

  bool ok=rh_init();
  if(!ok) return false;

  radio_read_buf  = chunkbuf_new(RADIO_READ_BUFFER_SIZE,  true);
  radio_write_buf = chunkbuf_new(RADIO_WRITE_BUFFER_SIZE, true);

  initialised=true;

  if(recv_cb) recv_cb(true, "radio");

  return true;
}

#define NL_DELIM '\n'

// -------------------------------------------------------------

static volatile uint64_t send_another_chunk_req=0;
static          void     send_another_chunk();

#if defined(PICO_RP2040)
#define RADIO_RFM69_DELAY_BETWEEN_CHUNK_SENDS_US 50
#elif defined(PICO_RP2350)
#define RADIO_RFM69_DELAY_BETWEEN_CHUNK_SENDS_US 500
#endif

uint16_t radio_available(){
  if(!initialised) return 0;
  if(send_another_chunk_req &&
     (time_us() > send_another_chunk_req + RADIO_RFM69_DELAY_BETWEEN_CHUNK_SENDS_US)){
    send_another_chunk_req=0;
    send_another_chunk();
  }
  return chunkbuf_current_size(radio_read_buf);
}

static volatile int8_t last_rssi = -127;

int8_t radio_last_rssi(){
  return last_rssi;
}

static void on_receive(uint8_t* buf, uint16_t len){
  if(!chunkbuf_writable(radio_read_buf, len, -1)){
    log_write("rrb full %d %d\n", len, chunkbuf_current_size(radio_read_buf));
    log_flash(1,0,0);
    return;
  }
  chunkbuf_write(radio_read_buf, (char*)buf, len, -1);
  if(recv_cb) recv_cb(false, "radio");
}

int16_t radio_read(char* buf, uint16_t len){
  if(!initialised) return 0;
  uint16_t r=chunkbuf_readable(radio_read_buf, NL_DELIM);

#ifdef TRACK_CHUNK_ASSEMBLY
  static uint16_t l_a=0;
  uint16_t a=chunkbuf_current_size(radio_read_buf);
  if(a!=l_a){
    l_a=a;
    chunkbuf_dump(radio_read_buf);
  }
#endif

  if(!r) return 0;
  if(r > len){
    log_flash(1,0,0); // can fill whole buffer without seeing delim
    log_write("**** %d > %d\n", r, len);
    return 0;
  }
  return chunkbuf_read(radio_read_buf, buf, len, NL_DELIM);
}

// ---------------------------

static volatile bool write_loop_in_progress=false;

static void on_tx_write_chunk_1st(){
  if(write_loop_in_progress) return;
  log_write("=== tx start ===\n");
  write_loop_in_progress=true;
  send_another_chunk_req=time_us();
}

static void on_tx_write_chunk_ongoing(){
  if(chunkbuf_current_size(radio_write_buf)){
    send_another_chunk_req=time_us();
  }
  else{
    write_loop_in_progress = false;
    rh_set_mode_rx();
    log_write("=== tx end ===\n");
  }
}

static void send_another_chunk(){
  rh_set_mode_standby();
  char buf[128];
  int16_t len = chunkbuf_read(radio_write_buf, buf, RFM69_MAX_MESSAGE_LEN-1, -1);
                                                     // REVISIT scared of Off-By-One!
                                                     // REVISIT why not 128 bytes?
  rh_write_fifo((uint8_t*)buf, len);
  rh_set_mode_tx();
}

uint16_t radio_write(char* band, char* buf, uint16_t len) {
  radio_wake();

  if(!chunkbuf_writable(radio_write_buf, len, NL_DELIM)){
    log_flash(1,0,0); // no room for this len
    return 0;
  }
  chunkbuf_write(radio_write_buf, buf, len, NL_DELIM);
  on_tx_write_chunk_1st();

  return len;
}

// ---------------------------

void radio_sleep(){
  rh_sleep();
}

void radio_wake(){
}

// ---------------------------------------------------------------------------------

/*
Code modified from RadioHead Copyright 2011-2014 Mike McCauley (mikem@airspayce.com)

If you wish to use this software under Open Source Licensing, you must contribute all
your source code to the open source community in accordance with the GPL Version 2 when
your application is distributed. See http://www.gnu.org/copyleft/gpl.html

We try hard to keep it up to date, fix bugs and to provide free support. If this library
has helped you save time or money, please consider donating at http://www.airspayce.com

RadioHead is a trademark of AirSpayce Pty Ltd. The RadioHead mark was first used on
April 12 2014 for international trade, and is used only in relation to data
communications hardware and software and related services.  It is not to be confused
with any other similar marks covering other goods and services.
*/

#include "radio-rfm69.h" // pages and pages from the datasheet

#ifdef RFM69_CS_PIN
static const uint8_t radio_rfm69_cs_pin  = RFM69_CS_PIN;
static const uint8_t radio_rfm69_rst_pin = RFM69_RST_PIN;
static const uint8_t radio_rfm69_int_pin = RFM69_INT_PIN;
#else
extern const uint8_t radio_rfm69_cs_pin;
extern const uint8_t radio_rfm69_rst_pin;
extern const uint8_t radio_rfm69_int_pin;
#endif

typedef enum {
  rh_initialising = 0,
  rh_sleeping,
  rh_standby,
  rh_transmitting,
  rh_listening
} rh_mode;

static volatile rh_mode mode = rh_initialising;

static int8_t rh_power = 0;

static uint8_t rh_spi_read_register(uint8_t reg) {
  uint8_t val;
  TIMING_CRITICAL_ENTER;
  gpio_set(radio_rfm69_cs_pin, 0);
  spi_rw_byte(reg & ~RFM69_SPI_WRITE_MASK);
  val = spi_rw_byte(0);
  time_delay_us(1);
  gpio_set(radio_rfm69_cs_pin, 1);
  TIMING_CRITICAL_END;
  return val;
}

static void rh_spi_write_register(uint8_t reg, uint8_t val) {
  TIMING_CRITICAL_ENTER;
  gpio_set(radio_rfm69_cs_pin, 0);
  spi_rw_byte(reg | RFM69_SPI_WRITE_MASK);
  spi_rw_byte(val);
  time_delay_us(1);
  gpio_set(radio_rfm69_cs_pin, 1);
  TIMING_CRITICAL_END;
}

static uint16_t rh_spi_read(uint8_t reg, uint8_t* buf, uint8_t len) {
  TIMING_CRITICAL_ENTER;
  gpio_set(radio_rfm69_cs_pin, 0);
  spi_rw_byte(reg & ~RFM69_SPI_WRITE_MASK);
  uint16_t n=spi_read(buf, len);
  time_delay_us(1);
  gpio_set(radio_rfm69_cs_pin, 1);
  TIMING_CRITICAL_END;
  return n;
}

static void rh_spi_write(uint8_t reg, const uint8_t* buf, uint8_t len) {
  TIMING_CRITICAL_ENTER;
  gpio_set(radio_rfm69_cs_pin, 0);
  spi_rw_byte(reg | RFM69_SPI_WRITE_MASK);
  spi_write(buf, len);
  time_delay_us(1);
  gpio_set(radio_rfm69_cs_pin, 1);
  TIMING_CRITICAL_END;
}

#define RADIO_BROADCAST_ADDRESS 0xff

static void rh_write_fifo(uint8_t* buf, size_t len) {
#ifdef TRACK_CHUNK_ASSEMBLY
  log_write("rh_write_fifo len=%d [%.*s]\n", len, len, buf);
#endif
  TIMING_CRITICAL_ENTER;
  gpio_set(radio_rfm69_cs_pin, 0);
  spi_rw_byte(RFM69_REG_00_FIFO | RFM69_SPI_WRITE_MASK);
  spi_rw_byte(len);
  spi_write(buf, len);
  time_delay_us(1);
  gpio_set(radio_rfm69_cs_pin, 1);
  TIMING_CRITICAL_END;
}

static void rh_read_fifo() {
  TIMING_CRITICAL_ENTER;
  gpio_set(radio_rfm69_cs_pin, 0);
  spi_rw_byte(RFM69_REG_00_FIFO & ~RFM69_SPI_WRITE_MASK);
  uint8_t len = spi_rw_byte(0);
  if(len <= RFM69_MAX_MESSAGE_LEN) {
    uint8_t buf[128];
    spi_read(buf,len);
    last_rssi = -((int8_t)(rh_spi_read_register(RFM69_REG_24_RSSIVALUE) >> 1));
#ifdef TRACK_CHUNK_ASSEMBLY
    log_write("rh_read_fifo len=%d [%.*s]\n", len, len, buf);
#endif
    on_receive(buf,len);
  }
  time_delay_us(1);
  gpio_set(radio_rfm69_cs_pin, 1);
  TIMING_CRITICAL_END;
}

static void rh_set_opmode(uint8_t rfm_opmode) {
  uint8_t opmode = rh_spi_read_register(RFM69_REG_01_OPMODE);
  opmode &= ~RFM69_OPMODE_MODE;
  opmode |= (rfm_opmode & RFM69_OPMODE_MODE);
  rh_spi_write_register(RFM69_REG_01_OPMODE, opmode);
  while(!(rh_spi_read_register(RFM69_REG_27_IRQFLAGS1) & RFM69_IRQFLAGS1_MODEREADY));
}

static void rh_set_mode_standby() {
  if (mode == rh_standby) return;
  if (rh_power >= 18) {
    rh_spi_write_register(RFM69_REG_5A_TESTPA1, RFM69_TESTPA1_NORMAL);
    rh_spi_write_register(RFM69_REG_5C_TESTPA2, RFM69_TESTPA2_NORMAL);
  }
  rh_set_opmode(RFM69_OPMODE_MODE_STDBY);
  mode = rh_standby;
}

static void rh_set_mode_rx() {
  if (mode == rh_listening) return;
  if (rh_power >= 18) {
    rh_spi_write_register(RFM69_REG_5A_TESTPA1, RFM69_TESTPA1_NORMAL);
    rh_spi_write_register(RFM69_REG_5C_TESTPA2, RFM69_TESTPA2_NORMAL);
  }
  rh_spi_write_register(RFM69_REG_25_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0MAPPING_01); // payload ready
  rh_set_opmode(RFM69_OPMODE_MODE_RX);
  mode = rh_listening;
}

static void rh_set_mode_tx() {
  if (mode == rh_transmitting) return;
  if (rh_power >= 18) {
    rh_spi_write_register(RFM69_REG_5A_TESTPA1, RFM69_TESTPA1_BOOST);
    rh_spi_write_register(RFM69_REG_5C_TESTPA2, RFM69_TESTPA2_BOOST);
  }
  rh_spi_write_register(RFM69_REG_25_DIOMAPPING1, RFM69_DIOMAPPING1_DIO0MAPPING_00); // packet sent
  rh_set_opmode(RFM69_OPMODE_MODE_TX);
  mode = rh_transmitting;
}

static void rh_handle_interrupt(uint8_t pin, uint8_t edge) {

  uint8_t irqflags2 = rh_spi_read_register(RFM69_REG_28_IRQFLAGS2);

  if(mode == rh_transmitting) {
    bool packet_sent = (irqflags2 & RFM69_IRQFLAGS2_PACKETSENT);
    if(packet_sent && write_loop_in_progress) on_tx_write_chunk_ongoing();
  }
  else
  if(mode == rh_listening){
    bool crc_ok        = (irqflags2 & RFM69_IRQFLAGS2_CRCOK);
    bool payload_ready = (irqflags2 & RFM69_IRQFLAGS2_PAYLOADREADY);
    if(!crc_ok) {
      log_write("CRC fail\n");
      #define CRC_MAGIC "{ banana: 🍌 }\n"
      uint8_t len = strlen(CRC_MAGIC);
      uint8_t buf[len+1];
      mem_strncpy((char*)buf, CRC_MAGIC, len+1);
      on_receive(buf,len);
      return;
    }
    if(payload_ready) rh_read_fifo();
  }
}

static void rh_set_sync_words(const uint8_t* syncWords, uint8_t len) {
  uint8_t syncconfig = rh_spi_read_register(RFM69_REG_2E_SYNCCONFIG);
  if (syncWords && len && len <= 4) {
    rh_spi_write(RFM69_REG_2F_SYNCVALUE1, syncWords, len);
    syncconfig |= RFM69_SYNCCONFIG_SYNCON;
  } else {
    syncconfig &= ~RFM69_SYNCCONFIG_SYNCON;
  }
  syncconfig &= ~RFM69_SYNCCONFIG_SYNCSIZE;
  syncconfig |= (len-1) << 3;
  rh_spi_write_register(RFM69_REG_2E_SYNCCONFIG, syncconfig);
}

static void rh_set_modem_registers(const modem_config* config) {
  rh_spi_write(RFM69_REG_02_DATAMODUL,     &config->reg_02, 5);
  rh_spi_write(RFM69_REG_19_RXBW,          &config->reg_19, 2);
  rh_spi_write_register(      RFM69_REG_37_PACKETCONFIG1,  config->reg_37);
}

static bool rh_set_modem_config(modem_config_choice index) {
  if (index > (signed int)(sizeof(MODEM_CONFIG_TABLE) / sizeof(modem_config))) return false;
  modem_config cfg;
  memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(modem_config));
  rh_set_modem_registers(&cfg);
  return true;
}

static void rh_set_preamble(uint16_t bytes) {
  rh_spi_write_register(RFM69_REG_2C_PREAMBLEMSB, bytes >> 8);
  rh_spi_write_register(RFM69_REG_2D_PREAMBLELSB, bytes & 0xff);
}

#define RFM69_FXOSC 32000000.0
#define RFM69_FSTEP  (RFM69_FXOSC / 524288)

static bool rh_set_frequency(float centre) {
  uint32_t frf = (uint32_t)((centre * 1000000.0) / RFM69_FSTEP);
  rh_spi_write_register(RFM69_REG_07_FRFMSB, (frf >> 16) & 0xff);
  rh_spi_write_register(RFM69_REG_08_FRFMID, (frf >> 8) & 0xff);
  rh_spi_write_register(RFM69_REG_09_FRFLSB, frf & 0xff);
  return true;
}

static void rh_set_encryption_key(uint8_t* key) {
  if (key) {
    rh_spi_write(RFM69_REG_3E_AESKEY1, key, 16);
    rh_spi_write_register(RFM69_REG_3D_PACKETCONFIG2,
                   rh_spi_read_register(RFM69_REG_3D_PACKETCONFIG2) | RFM69_PACKETCONFIG2_AESON);
  } else {
    rh_spi_write_register(RFM69_REG_3D_PACKETCONFIG2,
                   rh_spi_read_register(RFM69_REG_3D_PACKETCONFIG2) & ~RFM69_PACKETCONFIG2_AESON);
  }
}

static void rh_set_tx_power(int8_t p, bool ishighpowermodule) {

  rh_power = p;
  uint8_t palevel;

  if(ishighpowermodule) {
    if(rh_power < -2){
      rh_power = -2; // RFM69HW only works down to -2
    }
    if(rh_power <= 13) {
      //Need PA1 exclusivelly on RFM69HW
      palevel = RFM69_PALEVEL_PA1ON
              | ((rh_power + 18) & RFM69_PALEVEL_OUTPUTPOWER);
    } else
    if(rh_power >= 18) {
      // Need PA boost settings change when tx is turned on and off,
      palevel = RFM69_PALEVEL_PA1ON
              | RFM69_PALEVEL_PA2ON
              | ((rh_power + 11) & RFM69_PALEVEL_OUTPUTPOWER);
    } else { // +14dBm to +17dBm
      palevel = RFM69_PALEVEL_PA1ON
              | RFM69_PALEVEL_PA2ON
              | ((rh_power + 14) & RFM69_PALEVEL_OUTPUTPOWER);
    }
  } else {
    if (rh_power < -18) rh_power = -18;
    if (rh_power >  13) rh_power =  13; //limit for RFM69W
    palevel = RFM69_PALEVEL_PA0ON | ((rh_power + 18) & RFM69_PALEVEL_OUTPUTPOWER);
  }
  rh_spi_write_register(RFM69_REG_11_PALEVEL, palevel);
}

static int8_t rh_read_temperature() {
  rh_spi_write_register(     RFM69_REG_4E_TEMP1,   RFM69_TEMP1_TEMPMEASSTART);
  while(rh_spi_read_register(RFM69_REG_4E_TEMP1) & RFM69_TEMP1_TEMPMEASRUNNING);
  return (166 - rh_spi_read_register(RFM69_REG_4F_TEMP2));
}

static void rh_sleep() {
  if(mode==rh_sleeping) return;
  rh_spi_write_register(RFM69_REG_01_OPMODE, RFM69_OPMODE_MODE_SLEEP);
  mode = rh_sleeping;
}

static void rh_reset(){
  gpio_set(radio_rfm69_rst_pin, 1); time_delay_ms(10);
  gpio_set(radio_rfm69_rst_pin, 0); time_delay_ms(10);
}

static bool rh_init() {

  spi_init();

  gpio_mode(radio_rfm69_cs_pin, GPIO_MODE_OUTPUT);
  gpio_set( radio_rfm69_cs_pin, 1);

  gpio_mode(radio_rfm69_rst_pin, GPIO_MODE_OUTPUT);
  time_delay_ms(10); // REVISIT: have to leave floating?
  gpio_set( radio_rfm69_rst_pin, 0);
  time_delay_ms(10);
  rh_reset();

  // REVISIT: setting irq priority
  gpio_mode_cb(radio_rfm69_int_pin, GPIO_MODE_INPUT_PULLDOWN, GPIO_RISING, rh_handle_interrupt);

  uint8_t this_device_version = rh_spi_read_register(RFM69_REG_10_VERSION);
  log_write("version number of rfm69 module=%#x\n", this_device_version);
  if (this_device_version == 00 || this_device_version == 0xff) return false;

  rh_set_mode_standby();

  rh_spi_write_register(RFM69_REG_3C_FIFOTHRESH, RFM69_FIFOTHRESH_TXSTARTCONDITION_NOTEMPTY | 0x0f);
  rh_spi_write_register(RFM69_REG_29_RSSITHRESH, 220); // -110 dbM
  rh_spi_write_register(RFM69_REG_2E_SYNCCONFIG, RFM69_SYNCCONFIG_SYNCON); // auto, tolerance 0
  rh_spi_write_register(RFM69_REG_38_PAYLOADLENGTH, RFM69_FIFO_SIZE); // max size only for RX
  rh_spi_write_register(RFM69_REG_6F_TESTDAGC, RFM69_TESTDAGC_CONTINUOUSDAGC_IMPROVED_LOWBETAOFF);
  rh_spi_write_register(RFM69_REG_5A_TESTPA1, RFM69_TESTPA1_NORMAL);
  rh_spi_write_register(RFM69_REG_5C_TESTPA2, RFM69_TESTPA2_NORMAL);

  uint8_t syncwords[] = { 0x2d, 0xd4 };
  rh_set_sync_words(syncwords, sizeof(syncwords));

  rh_set_modem_config(GFSK_Rb250Fd250); // 2 CRC CCITT octets computed on the length and data

  rh_set_preamble(4); // REVISIT
  rh_set_frequency(915.0); // REVISIT: config
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // REVISIT: config
  rh_set_encryption_key(key);
  rh_set_tx_power(20, true);  // REVISIT: config

  return true;
}






