
#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/binary_info.h>

#include <hardware/i2c.h>

#include <onx/log.h>
#include <onx/time.h>
#include <onx/i2c.h>
#undef i2c_init

bool initialised=false;
bool initialised_2=false;

void* i2c_init_avoid_sdk(uint16_t speed_khz) {

  if(initialised) return (void*)i2c_default;

  i2c_init(i2c_default, speed_khz * 1000);

  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

  initialised=true;
  return (void*)i2c_default;
}

#define NON_DEFAULT_I2C (i2c_default==i2c0? i2c1: i2c0)

void* i2c_init_2(uint16_t speed_khz, uint8_t sda_pin, uint8_t scl_pin){

  if(initialised_2) return (void*)NON_DEFAULT_I2C;

  i2c_init(NON_DEFAULT_I2C, speed_khz * 1000);

  gpio_set_function(sda_pin, GPIO_FUNC_I2C);
  gpio_set_function(scl_pin, GPIO_FUNC_I2C);

  gpio_pull_up(sda_pin);
  gpio_pull_up(scl_pin);

//bi_decl(bi_2pins_with_func(sda_pin, scl_pin, GPIO_FUNC_I2C));

  initialised_2=true;
  return (void*)NON_DEFAULT_I2C;
}

uint8_t i2c_read(void* i2c_inst, uint8_t addr, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = i2c_read_blocking((i2c_inst_t*)i2c_inst, addr, buf, len, false);
  return n<=0;
}

uint8_t i2c_write(void* i2c_inst, uint8_t addr, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, buf, len, false);
  return n<=0;
}

uint8_t i2c_read_register(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, &reg, 1, true);
  if(n<=0) return 1;
  time_delay_us(800);
  n = i2c_read_blocking((i2c_inst_t*)i2c_inst, addr, buf, len, false);
  if(n<=0) return 1;
  time_delay_us(300);
  return 0;
}

uint8_t i2c_write_register(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {
  i2c_wake();
  int16_t n;
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, &reg, 1, true);
  if(n<=0) return 1;
  time_delay_us(800);
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, buf, len, false);
  if(n<=0) return 1;
  time_delay_us(300);
  return 0;
}

uint8_t i2c_write_register_byte(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t val) {
  i2c_wake();
  uint8_t buf[2] = { reg, val };
  int16_t n;
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, buf, 2, false);
  if(n<=0) return 1;
  time_delay_us(300);
  return 0;
}

uint8_t i2c_read_register_hi_lo(void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                                 uint8_t reg_lo, uint8_t* buf, uint16_t len){
  i2c_wake();

  uint8_t reg[] = { reg_hi, reg_lo };

  int16_t n;
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, reg, 2, false);
  if(n<=0) return 1;
  time_delay_us(250); // see ~/Sources/Adafruit_Seesaw/Adafruit_seesaw.h read(.. delay) REVISIT: put as arg?
  n = i2c_read_blocking((i2c_inst_t*)i2c_inst, addr, buf, len, false);
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
  n = i2c_write_blocking((i2c_inst_t*)i2c_inst, addr, b, 2+len, false);
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

