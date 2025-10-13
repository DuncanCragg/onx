#define NON_BLOCKING

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <onx/log.h>
#include <onx/mem.h>
#include <onx/radio.h>
#include <onx/chunkbuf.h>

#define RADIO_READ_BUFFER_SIZE  2048
#define RADIO_WRITE_BUFFER_SIZE 2048

#define RADIO_TXPOWER                  RADIO_TXPOWER_TXPOWER_0dBm
#define RADIO_CHANNEL                  7 // 2.407GHz

#define RADIO_BASE_ADDRESS             0x75626974
#define RADIO_DEFAULT_GROUP            0

#define RADIO_MAX_PACKET_SIZE          252

static char rx_buffer[256];

static volatile bool initialised=false;
static volatile bool sleeping=false;

static volatile channel_recv_cb recv_cb = 0;

// REVISIT: when do I need chunkbuf_clear(radio_write_buf);

static volatile chunkbuf* radio_read_buf = 0;
static volatile chunkbuf* radio_write_buf = 0;

static void switch_to_tx();
static void switch_to_rx();
static bool write_a_packet(uint16_t size);

static volatile bool write_loop_in_progress=false;

static void do_tx_write_block(bool first_write){

  if(first_write && write_loop_in_progress) return;

  switch_to_tx();

  uint16_t size = chunkbuf_read(radio_write_buf, rx_buffer+1, RADIO_MAX_PACKET_SIZE, -1);

  if(!write_a_packet(size)){
    switch_to_rx();
  }
}

static void switch_to_tx(){

  if(write_loop_in_progress) return;
  write_loop_in_progress=true;

//PORT DisableIRQ(RADIO_IRQn);
//PORT turn off radio

//// go to START per packet, not on READY, do; no RSSI

//PORT enable TX

#ifdef NON_BLOCKING
//PORT ClearPendingIRQ(RADIO_IRQn); // REVISIT
//PORT EnableIRQ(RADIO_IRQn);
#endif
}

static void switch_to_rx(){

  if(!write_loop_in_progress) return;
  write_loop_in_progress = false;

#ifdef NON_BLOCKING
//PORT DisableIRQ(RADIO_IRQn);
#endif

//PORT turn off radio

//PORT enable RX

//PORT ClearPendingIRQ(RADIO_IRQn); // REVISIT
//PORT EnableIRQ(RADIO_IRQn);
}

static bool write_a_packet(uint16_t size){

  if(!size) return false;

  rx_buffer[0]=size;  // first byte is the size (!)

//PORT do it

#ifndef NON_BLOCKING
//PORT block
  do_tx_write_block(false);
#endif

  return true;
}

bool radio_init(list* bands, channel_recv_cb cb){

  recv_cb = cb;

  if(initialised) return true;

  radio_read_buf  = chunkbuf_new(RADIO_READ_BUFFER_SIZE, true);
  radio_write_buf = chunkbuf_new(RADIO_WRITE_BUFFER_SIZE, true);

//PORT set tx power, channel, bandwidth/mbs address crc white
//PORT set (uint32_t)rx_buffer;
//PORT set irq and priority
//// REVISIT: call switch_to_rx here?
//PORT start RX

  initialised=true;

  if(recv_cb) recv_cb(true, "radio");

  return true;
}

#define NL_DELIM '\n'

int16_t radio_read(char* buf, uint16_t size){
  if(!initialised) return 0;
  uint16_t r=chunkbuf_readable(radio_read_buf, NL_DELIM);
  if(!r) return 0;
  if(r > size){
    log_flash(1,0,0); // can fill whole buffer without seeing delim
    log_write("**** %d > %d\n", r, size);
    return 0;
  }
  uint16_t rr=chunkbuf_read(radio_read_buf, buf, size, NL_DELIM);
  return rr? rr: -1;
}

uint16_t radio_write(char* band, char* buf, uint16_t size) {
  radio_wake();
  if(!chunkbuf_writable(radio_write_buf, size, NL_DELIM)){
    log_flash(1,0,0); // no room for this size
    return 0;
  }
  chunkbuf_write(radio_write_buf, buf, size, NL_DELIM);
  do_tx_write_block(true);
  return size;
}

static int8_t last_rssi = -127;

int8_t radio_last_rssi(){
  return last_rssi;
}

uint16_t radio_available(){
  if(!initialised) return 0;
  return chunkbuf_current_size(radio_read_buf);
}

static void received(char* buf, uint16_t size, int8_t rssi){
  last_rssi=rssi;
  if(!chunkbuf_writable(radio_read_buf, size, -1)){
    log_write("rrb full %d %d\n", size, chunkbuf_current_size(radio_read_buf));
    //log_flash(1,0,0);
    return;
  }
  chunkbuf_write(radio_read_buf, buf, size, -1);
  if(recv_cb) recv_cb(false, "radio");
}

void RADIO_IRQHandler(void){

#ifdef NON_BLOCKING
  if(write_loop_in_progress) {
//  if(){ PORT
      do_tx_write_block(false);
//  }
    return;
  }
#endif

//PORT if read data
  {
    uint8_t size;
//  PORT
//  if(CRC OK) {
//    size = rx_buffer[0];
//  }
//  else{
      #define CORRUPTION_MAGIC "{ banana: 🍌 }\n"
      size = strlen(CORRUPTION_MAGIC);
      mem_strncpy(rx_buffer+1, CORRUPTION_MAGIC, size+1);
//  }
    int8_t rssi = 11; // PORT get RSSI
    received(rx_buffer+1, size, rssi);
  }
}

void radio_sleep(){
  if(sleeping) return;
  sleeping=true;
}

void radio_wake(){
  if(!sleeping) return;
  sleeping=false;
}




