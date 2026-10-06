#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void*   i2c_init();
uint8_t i2c_read(          void* i2c_inst, uint8_t addr,                 uint8_t* buf, uint16_t len);
uint8_t i2c_write(         void* i2c_inst, uint8_t addr,                 uint8_t* buf, uint16_t len);
uint8_t i2c_read_reg(      void* i2c_inst, uint8_t addr, uint8_t reg,    uint8_t* buf, uint16_t len, bool autoinc);
uint8_t i2c_write_reg(     void* i2c_inst, uint8_t addr, uint8_t reg,    uint8_t* buf, uint16_t len);
uint8_t i2c_write_reg_byte(void* i2c_inst, uint8_t addr, uint8_t reg,    uint8_t  val);
uint8_t i2c_read_reg_hilo( void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                         uint8_t reg_lo, uint8_t* buf, uint16_t len, uint16_t dus);
uint8_t i2c_write_reg_hilo(void* i2c_inst, uint8_t addr, uint8_t reg_hi,
                                                         uint8_t reg_lo, uint8_t* buf, uint16_t len);
void    i2c_sleep();
void    i2c_wake();

// * dus = delay us; 80 OK, 50 not, 150 safe, 500 for ADC

#endif
