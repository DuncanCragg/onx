
// https://www.newhavendisplay.com/appnotes/datasheets/LCDs/ST7789V.pdf

#include "sdk_common.h"

#include "boards.h"

#include <onx/gpio.h>
#include <onx/time.h>
#include <onx/log.h>
#include <onx/spi.h>
#include <onx/display.h>

uint16_t screen_width  = ST7789_WIDTH;
uint16_t screen_height = ST7789_HEIGHT;

#define ST7789_NOP         0x00
#define ST7789_SWRESET     0x01
#define ST7789_RDDID       0x04
#define ST7789_RDDST       0x09

#define ST7789_SLPIN       0x10
#define ST7789_SLPOUT      0x11
#define ST7789_PTLON       0x12
#define ST7789_NORON       0x13
#define ST7789_TEON        0x35
#define ST7789_TESCAN      0x44

#define ST7789_RDMODE      0x0A
#define ST7789_RDMADCTL    0x0B
#define ST7789_RDPIXFMT    0x0C
#define ST7789_RDIMGFMT    0x0D
#define ST7789_RDSELFDIAG  0x0F

#define ST7789_INVOFF      0x20
#define ST7789_INVON       0x21
#define ST7789_GAMMASET    0x26
#define ST7789_DISPOFF     0x28
#define ST7789_DISPON      0x29

#define ST7789_CASET       0x2A
#define ST7789_RASET       0x2B
#define ST7789_RAMWR       0x2C
#define ST7789_RAMRD       0x2E

#define ST7789_PTLAR       0x30
#define ST7789_MADCTL      0x36
#define ST7789_COLMOD      0x3A

#define ST7789_FRMCTR1     0xB1
#define ST7789_RGBCTRL     0xB1
#define ST7789_PORCTR      0xB2
//#define ST7789_FRMCTR2     0xB2
#define ST7789_FRMCTR3     0xB3
#define ST7789_INVCTR      0xB4
#define ST7789_DFUNCTR     0xB6
#define ST7789_GCTRL       0xB7

#define ST7789_VCOM        0xBB

#define ST7789_LCMCTR      0xC0
//#define ST7789_PWCTR1      0xC0
#define ST7789_PWCTR2      0xC1
#define ST7789_VDVVRHEN    0xC2
//#define ST7789_PWCTR3      0xC2
#define ST7789_VRHS        0xC3
//#define ST7789_PWCTR4      0xC3
#define ST7789_VDVS        0xC4
//#define ST7789_PWCTR5      0xC4
#define ST7789_VMCTR1      0xC5
#define ST7789_FRMCTR2     0xC6
#define ST7789_VMCTR2      0xC7
#define ST7789_PWCTRSEQ    0xCB
#define ST7789_PWCTRA      0xCD
#define ST7789_PWCTRB      0xCF

#define ST7789_PWCTRL1     0xD0

#define ST7789_RDID1       0xDA
#define ST7789_RDID2       0xDB
#define ST7789_RDID3       0xDC
#define ST7789_RDID4       0xDD

#define ST7789_PVGAMCTRL     0xE0
#define ST7789_NVGAMCTRL     0xE1
#define ST7789_DGMCTR1     0xE2
#define ST7789_DGMCTR2     0xE3
#define ST7789_SPI2EN      0xE7
#define ST7789_TIMCTRA     0xE8
#define ST7789_TIMCTRB     0xEA

#define ST7789_ENGMCTR     0xF2
#define ST7789_INCTR       0xF6
#define ST7789_PUMP        0xF7

#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00 //replace BGR with this to get normal coloring
#define ST7789_MADCTL_BGR 0x08 //used by default
#define ST7789_MADCTL_MH  0x04

// ------------------------------------------

uint64_t time_ready_after_wake_command=0;

static void wait_for_display_to_settle_after_wake(){
  if(time_ready_after_wake_command){
    int32_t time_till_ready=time_ready_after_wake_command-time_ms();
    if(time_till_ready>0) time_delay_ms(time_till_ready);
    time_ready_after_wake_command=0;
  }
}

static bool sleeping=false;

// ------------------------------------------

void start_write(void) {
  if(sleeping) return;
  spi_enable(true);
  gpio_set(SPIM_SS_PIN , 0);
}

void end_write(void) {
  if(sleeping) return;
  gpio_set(SPIM_SS_PIN , 1);
  spi_enable(false);
}

void write_command(uint8_t d) {
  if(sleeping) return;
  gpio_set(ST7789_DC_PIN , 0);
  spi_write(&d, 1);
  gpio_set(ST7789_DC_PIN , 1);
}

void write_char(uint8_t d) {
  if(sleeping) return;
  spi_write(&d, 1);
}

static void set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

  if(sleeping) return;
  wait_for_display_to_settle_after_wake();

#if defined(ST7789_ADDR_HEIGHT)
  y = y + (ST7789_ADDR_HEIGHT - ST7789_HEIGHT);
#endif
  write_command(ST7789_CASET);
  write_char((x) >> 8);
  write_char(x);
  write_char((x + w - 1) >> 8);
  write_char(x + w - 1);
  write_command(ST7789_RASET);
  write_char((y) >> 8);
  write_char(y);
  write_char(((y + h - 1) ) >> 8);
  write_char((y + h - 1) );
  write_command(ST7789_RAMWR);
}

void display_reset_kinda_slow() {
  time_delay_ms(20);
  gpio_set(ST7789_RST_PIN, 0);
  time_delay_ms(100);
  gpio_set(ST7789_RST_PIN, 1);
  time_delay_ms(100);
}

static void st7789_rotation_set(uint16_t rotation) {

  if(sleeping) return;
  wait_for_display_to_settle_after_wake();

  write_command(ST7789_MADCTL);

  switch(rotation) {
      case 0:
          write_char(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
          break;
      case 90:
          write_char(ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
          break;
      case 180:
          write_char(ST7789_MADCTL_RGB);
          break;
      case 270:
          write_char(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
          break;
      default:
          break;
  }
}

void init_command_list() {

  sleeping=false;

  start_write();

  write_command(ST7789_SLPOUT);
  time_delay_ms(120);

  st7789_rotation_set(180);

  write_command(ST7789_COLMOD);
  write_char(0x55); //16-bit, 565 RGB ATC had 5?
/*
  write_command(ST7789_PORCTR);
  write_char(0xB);
  write_char(0xB);
  write_char(0x33);
  write_char(0x0);
  write_char(0x33);

  write_command(ST7789_GCTRL);
  write_char(0x11);

  write_command(ST7789_VCOM);
  write_char(0x35);

  write_command(ST7789_LCMCTR);
  write_char(0x2c);

  write_command(ST7789_VDVVRHEN);
  write_char(1);
  write_command(ST7789_VRHS);
  write_char(8);
  write_command(ST7789_VDVS);
  write_char(0x20);
  write_command(ST7789_FRMCTR2);
  write_char(0x1f);

  write_command(ST7789_PWCTRL1);
  write_char(0xa4);
  write_char(0xa1);

  write_command(ST7789_PVGAMCTRL);
  write_char(0xF0);
  write_char(0x04);
  write_char(0x0A);
  write_char(0x0A);
  write_char(0x08);
  write_char(0x25);
  write_char(0x33);
  write_char(0x27);
  write_char(0x3D);
  write_char(0x38);
  write_char(0x14);
  write_char(0x14);
  write_char(0x25);
  write_char(0x2A);

  write_command(ST7789_NVGAMCTRL);
  write_char(0xF0);
  write_char(0x05);
  write_char(0x08);
  write_char(0x07);
  write_char(0x06);
  write_char(0x02);
  write_char(0x26);
  write_char(0x32);
  write_char(0x3D);
  write_char(0x3A);
  write_char(0x16);
  write_char(0x16);
  write_char(0x26);
  write_char(0x2C);
*/
  write_command(ST7789_INVON);

  write_command(ST7789_TEON);
  write_char(0x0);

  write_command(ST7789_TESCAN);
  write_char(0x25);
  write_char(0x0);
  time_delay_ms(120);

  write_command(ST7789_DISPON);

  set_addr_window(0, 0, ST7789_WIDTH, ST7789_HEIGHT);

  end_write();
}

void display_init() {

  spi_init(SPIM_SCK_PIN, SPIM_MOSI_PIN, SPIM_MISO_PIN, SPIM_SS_PIN);

  gpio_mode(ST7789_DC_PIN,  GPIO_MODE_OUTPUT);
  gpio_mode(ST7789_RST_PIN, GPIO_MODE_OUTPUT);

  gpio_set(ST7789_DC_PIN , 1);
  gpio_set(ST7789_RST_PIN, 1);

  display_reset_kinda_slow();

  init_command_list();
}

void display_write_out_buffer(uint8_t* buf, uint32_t len) {

  if(sleeping) return;
  wait_for_display_to_settle_after_wake();

  start_write();
  spi_write(buf, len);
  end_write();
}

#define SPI_FLUSH_TIME           29
#define ST7789_WAKE_SETTLE_TIME 120

void display_sleep(bool hard) {

  if(sleeping) return;
  sleeping=true;

  if(hard){
    display_reset_kinda_slow();
    return;
  }
  write_command(ST7789_DISPOFF);
  write_command(ST7789_SLPIN);

  time_delay_ms(5);
}

void display_wake(bool hard) {

  if(!sleeping) return;
  sleeping=false;

  if(hard){
    init_command_list();
    return;
  }
  write_command(ST7789_SLPOUT);
  write_command(ST7789_DISPON);
  
  time_ready_after_wake_command=time_ms() + SPI_FLUSH_TIME + ST7789_WAKE_SETTLE_TIME;
}





