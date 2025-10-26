#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <onx/radio.h>

bool radio_init(channel_recv_cb cb){
  return true;
}

int16_t radio_read(char* buf, uint16_t len){
  return -1;
}

uint16_t radio_write(char* band, char* buf, uint16_t len) {
  return 0;
}

int8_t radio_last_rssi(){
  return 0;
}

uint16_t radio_available(){
  return 0;
}

void radio_sleep(){
}

void radio_wake(){
}




