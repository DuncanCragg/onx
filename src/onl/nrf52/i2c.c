
#include <nrfx_twi.h>
#include <boards.h>

#include <onx/log.h>
#include <onx/time.h>
#include <onx/i2c.h>

nrfx_twi_t i2c_inst = NRFX_TWI_INSTANCE(1);

bool initialized=false;

void* i2c_init() {
  if(initialized) return (void*)&i2c_inst;
  nrfx_twi_config_t i2c_config;
  i2c_config.frequency = I2C_SPEED_KHZ;
  i2c_config.scl       = I2C_SCL_PIN;
  i2c_config.sda       = I2C_SDA_PIN;
  i2c_config.interrupt_priority = NRFX_TWI_DEFAULT_CONFIG_IRQ_PRIORITY;
  i2c_config.hold_bus_uninit =    NRFX_TWI_DEFAULT_CONFIG_HOLD_BUS_UNINIT;
  nrfx_twi_init(&i2c_inst, &i2c_config, 0, 0);
  nrfx_twi_enable(&i2c_inst);
  initialized=true;
  return (void*)&i2c_inst;
}

uint8_t i2c_read(void* i2c_inst, uint8_t addr, uint8_t* buf, uint16_t len) {
  i2c_wake();
  ret_code_t e=nrfx_twi_rx((nrfx_twi_t*)i2c_inst, addr, buf, len);
  return e? 1: 0;
}

uint8_t i2c_write(void* i2c_inst, uint8_t addr, uint8_t* buf, uint16_t len) {
  i2c_wake();
  ret_code_t e=nrfx_twi_tx((nrfx_twi_t*)i2c_inst, addr, buf, len, false);
  return e? 1: 0;
}

uint8_t i2c_read_reg(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len, bool autoinc) {

  ret_code_t e;

  i2c_wake();

  uint8_t reg_80 = reg;
  if(len > 1 && autoinc) reg_80 |= 0x80;

  e=nrfx_twi_tx((nrfx_twi_t*)i2c_inst, addr, &reg_80, 1, true);
  if(e) return 1;

  e=nrfx_twi_rx((nrfx_twi_t*)i2c_inst, addr, buf, len);
  if(e) return 1;

  return 0;
}

uint8_t i2c_write_reg(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {

  ret_code_t e;

  i2c_wake();

  uint8_t reg_buf[1 + len];

  reg_buf[0] = reg;

  memcpy(&reg_buf[1], buf, len);

  e = nrfx_twi_tx((nrfx_twi_t*)i2c_inst, addr, reg_buf, sizeof(reg_buf), false);
  if(e) return 1;

  return 0;
}

uint8_t i2c_write_reg_byte(void* i2c_inst, uint8_t addr, uint8_t reg, uint8_t val) {

  ret_code_t e;

  i2c_wake();

  uint8_t reg_buf[2] = { reg, val };

  e=nrfx_twi_tx((nrfx_twi_t*)i2c_inst, addr, reg_buf, 2, false);
  if(e) return 1;

  return 0;
}

uint8_t i2c_read_reg_hilo(void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                        uint8_t reg_lo, uint8_t* buf, uint16_t len, uint16_t dus){
  ret_code_t e;

  i2c_wake();

  uint8_t reg[] = { reg_hi, reg_lo };

  e=nrfx_twi_tx((nrfx_twi_t*)i2c_inst, addr, reg, 2, false);
  if(e) return 1;

  time_delay_us(dus);

  e=nrfx_twi_rx((nrfx_twi_t*)i2c_inst, addr, buf, len);
  if(e) return 1;

  return 0;
}

uint8_t i2c_write_reg_hilo(void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                         uint8_t reg_lo, uint8_t* buf, uint16_t len){
  ret_code_t e;

  i2c_wake();

  uint8_t reg_buf[2 + len];

  reg_buf[0] = reg_hi;
  reg_buf[1] = reg_lo;

  memcpy(&reg_buf[2], buf, len);

  e=nrfx_twi_tx((nrfx_twi_t*)i2c_inst, addr, reg_buf, sizeof(reg_buf), false);
  if(e) return 1;

  return 0;
}

static bool sleeping=false;

void i2c_sleep() {
  if(sleeping) return;
  sleeping=true;
  NRF_TWI1->ENABLE=(TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos);
}

void i2c_wake() {
  if(!sleeping) return;
  sleeping=false;
  NRF_TWI1->ENABLE=(TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos);
}

//if(e) NRF_LOG_DEBUG("nrfx_twi addr=%x reg=%x val=%x err=%s", addr, reg, val, nrf_strerror_get(e)); log_loop();

