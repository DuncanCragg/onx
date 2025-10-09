#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void*   i2c_init_avoid_sdk(uint16_t speed_khz);
#define i2c_init i2c_init_avoid_sdk
uint8_t i2c_read(                void* i2c_inst, uint8_t address,                 uint8_t* buf, uint16_t len);
uint8_t i2c_write(               void* i2c_inst, uint8_t address,                 uint8_t* buf, uint16_t len);
uint8_t i2c_read_register(       void* i2c_inst, uint8_t address, uint8_t reg,    uint8_t* buf, uint16_t len);
uint8_t i2c_write_register(      void* i2c_inst, uint8_t address, uint8_t reg,    uint8_t* buf, uint16_t len);
uint8_t i2c_write_register_byte( void* i2c_inst, uint8_t address, uint8_t reg,    uint8_t  val);
uint8_t i2c_read_register_hi_lo( void* i2c_inst, uint8_t address, uint8_t reg_hi,
                                                                  uint8_t reg_lo, uint8_t* buf, uint16_t len);
uint8_t i2c_write_register_hi_lo(void* i2c_inst, uint8_t address, uint8_t reg_hi,
                                                                  uint8_t reg_lo, uint8_t* buf, uint16_t len);
void    i2c_sleep();
void    i2c_wake();

#endif
