
#include <math.h>

#include <boards.h>

#include <onx/log.h>
#include <onx/i2c.h>
#include <onx/time.h>
#include <onx/gpio.h>
#include <onx/motion.h>

#define SC7A20_REG_WHO_AM_I   0x0f
#define SC7A20_REG_CTRL_REG1  0x20
#define SC7A20_REG_CTRL_REG2  0x21
#define SC7A20_REG_CTRL_REG3  0x22
#define SC7A20_REG_CTRL_REG4  0x23
#define SC7A20_REG_CTRL_REG5  0x24
#define SC7A20_REG_CTRL_REG6  0x25
#define SC7A20_REG_STATUS     0x27
#define SC7A20_REG_X_LO       0x28
#define SC7A20_REG_X_HI       0x29
#define SC7A20_REG_Y_LO       0x2a
#define SC7A20_REG_Y_HI       0x2b
#define SC7A20_REG_Z_LO       0x2c
#define SC7A20_REG_Z_HI       0x2d

#define SC7A20_CHIP_ID        0x11

static void* i2c_inst;

static bool initialised=false;

static motion_state_t ms={0};

static void show_reg(char* name, uint8_t reg) {
  uint8_t val;
  uint8_t e = i2c_read_reg(i2c_inst, MOTION_ADDRESS, reg, &val, 1, false);
  if(!e) log_write("%s: %x\n",       name, val);
  else   log_write("%s: error %d\n", name, e);
}

bool motion_init() {

  if(initialised) return true;

  i2c_inst = i2c_init();

  uint8_t e;

  show_reg("SC7A20_REG_WHO_AM_I", SC7A20_REG_WHO_AM_I); // adds a delay or kick or something
  // needed at 100K?

  uint8_t chip_id=0;
  e=i2c_read_reg(i2c_inst, MOTION_ADDRESS, SC7A20_REG_WHO_AM_I, &chip_id, 1, false);
  if(e || chip_id != SC7A20_CHIP_ID){
    log_write("chip id err %x/%x", chip_id, SC7A20_CHIP_ID);
    return false;
  }
  log_write("LISnDH/SC7A20 found: id=%x\n", chip_id);

  // CTRL_REG1 (0x20): enable X/Y/Z, 100 Hz ODR
  uint8_t ctrl1 = 0x57;
  e = i2c_write_reg_byte(i2c_inst, MOTION_ADDRESS, SC7A20_REG_CTRL_REG1, ctrl1);
  if(e) {
    log_write("CTRL_REG1 write err %x", e);
    return false;
  }
  // CTRL_REG2 (0x21): disable high-pass filter
  uint8_t ctrl2 = 0x00;
  e = i2c_write_reg_byte(i2c_inst, MOTION_ADDRESS, SC7A20_REG_CTRL_REG2, ctrl2);
  if(e) {
    log_write("CTRL_REG2 write err %x", e);
    return false;
  }
  // CTRL_REG3 (0x22): disable interrupts
  uint8_t ctrl3 = 0x00;
  e = i2c_write_reg_byte(i2c_inst, MOTION_ADDRESS, SC7A20_REG_CTRL_REG3, ctrl3);
  if(e) {
    log_write("CTRL_REG3 write err %x", e);
    return false;
  }
  // CTRL_REG4 (0x23): BDU enabled, ±2g, high-resolution off
  uint8_t ctrl4 = 0x80;
  e = i2c_write_reg_byte(i2c_inst, MOTION_ADDRESS, SC7A20_REG_CTRL_REG4, ctrl4);
  if(e) {
    log_write("CTRL_REG4 write err %x", e);
    return false;
  }
  // CTRL_REG5 (0x24): FIFO and interrupt latches disabled
  uint8_t ctrl5 = 0x00;
  e = i2c_write_reg_byte(i2c_inst, MOTION_ADDRESS, SC7A20_REG_CTRL_REG5, ctrl5);
  if(e) {
    log_write("CTRL_REG5 write err %x", e);
    return false;
  }
  // CTRL_REG6 (0x25): disable interrupt routing
  uint8_t ctrl6 = 0x00;
  e = i2c_write_reg_byte(i2c_inst, MOTION_ADDRESS, SC7A20_REG_CTRL_REG6, ctrl6);
  if(e) {
    log_write("CTRL_REG6 write err %x", e);
    return false;
  }

  initialised=true;

  return true;
}

#define ONE_G 1060

motion_state_t motion_sample() {

  uint8_t e;

  uint8_t xyz[6] = {0};
  e = i2c_read_reg(i2c_inst, MOTION_ADDRESS, SC7A20_REG_X_LO, xyz, sizeof(xyz), true);
  if(e) {
    log_write("xyz read err %x", e);
    return ms;
  }
  uint16_t lsb = 0;
  uint16_t msb = 0;

  msb = xyz[1];
  lsb = xyz[0];

  ms.x = (int16_t)((msb << 8) | lsb);

  msb = xyz[3];
  lsb = xyz[2];

  ms.y = (int16_t)((msb << 8) | lsb);

  msb = xyz[5];
  lsb = xyz[4];

  ms.z = (int16_t)((msb << 8) | lsb);

  // 16bit +-32768=+-4g so /8=1024 per g (except 1g=ONE_G..?)
  ms.x /= 8;
  ms.y /= 8;
  ms.z /= 8;

  ms.m = ((int16_t)sqrtf(ms.x*ms.x+ms.y*ms.y+ms.z*ms.z))-ONE_G;

  return ms;
}

void motion_show_everything(){

  show_reg("SC7A20_REG_STATUS", SC7A20_REG_STATUS);

  motion_sample();

  log_write("x/y/z, m: %d/%d/%d, %d\n", ms.x,ms.y,ms.z, ms.m);
}













