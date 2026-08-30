
#include <stdio.h>

#include <onx/log.h>
#include <onx/time.h>
#include <onx/i2c.h>
#undef i2c_init

bool initialised=false;
bool initialised_2=false;

void* i2c_init_avoid_sdk(uint16_t speed_khz) {

  if(initialised) return (void*)0;

  initialised=true;
  return (void*)0;
}

void* i2c_init_2(uint16_t speed_khz, uint8_t sda_pin, uint8_t scl_pin){

  if(initialised_2) return (void*)0;

  initialised_2=true;
  return (void*)0;
}

uint8_t i2c_read(void* i2c_inst, uint8_t addr, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = 0;
  return n<=0;
}

uint8_t i2c_write(void* i2c_inst, uint8_t addr, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = 0;
  return n<=0;
}

uint8_t i2c_read_register(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = 0;
  if(n<=0) return 1;
  time_delay_us(800);
  n = 0;
  if(n<=0) return 1;
  time_delay_us(300);
  return 0;
}

uint8_t i2c_write_register(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = 0;
  if(n<=0) return 1;
  time_delay_us(800);
  n = 0;
  if(n<=0) return 1;
  time_delay_us(300);
  return 0;
}

uint8_t i2c_write_register_byte(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t val) {
  i2c_wake();
  uint8_t buf[2] = { reg, val };
  int16_t n;
  n = 0;
  if(n<=0) return 1;
  time_delay_us(300);
  return 0;
}

uint8_t i2c_read_register_hi_lo(void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                                 uint8_t reg_lo, uint8_t* buf, uint16_t len){
  i2c_wake();

  uint8_t reg[] = { reg_hi, reg_lo };

  int16_t n;
  n = 0;
  if(n<=0) return 1;
  time_delay_us(250);
  n = 0;
  if(n<=0) return 1;
  return 0;
}

uint8_t i2c_write_register_hi_lo(void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                                  uint8_t reg_lo, uint8_t* buf, uint16_t len){
  i2c_wake();

  uint8_t b[2+len];
  b[0]=reg_hi;
  b[1]=reg_lo;
  for(int i=0; i<len; i++) b[i+2]=buf[i];

  int16_t n;
  n = 0;
  if(n<=0) return 1;
  return 0;
}

static bool sleeping=false;

void i2c_sleep() {
  if(sleeping) return;
  sleeping=true;
  // PORT
}

void i2c_wake() {
  if(!sleeping) return;
  sleeping=false;
  // PORT
}

