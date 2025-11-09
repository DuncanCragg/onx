#ifndef HSTX_H
#define HSTX_H

// ------------------------------------------------------------------------------
// config for startup.c

#define HSTX_VREG_V      VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED  (300*1000)
#define HSTX_HSTXDIVISOR 2
/*
  240MHz = 90Hz or 60Hz @ 1024x600; 150MHz = 60Hz; 100MHz = 40Hz; 75MHz = 30Hz
  HSTX bit-rate per pin = 2 × 75 MHz = 150 Mbits/s
  TMDS uses 10 bit periods per channel symbol (CLKDIV=5 matches 10 bit periods)
  Pixel clock f_pixel = (2 × f_hstx) / 10 = f_hstx / 5 = 75 MHz / 5 = 15 MHz
  Frame rate = f_pixel / (total_pixels_per_frame) = 15,000,000 / 492,032 ≈ 30.487 Hz

  Tosh:
  1280x800 (0x5ef) 71.000MHz +HSync -VSync
        h: width  1280 start 1328 end 1360 total 1440 skew    0 clock  49.31KHz
        v: height  800 start  803 end  809 total  823           clock  59.91Hz

  WS 10.1":
  | Pixel Clock | H Blanking | V Blanking | H Front Porch | H Sync Width | H Image Size
  |    77.00    |    232     |     56     |      160      |      32      |     294
  | V Front Porch | V Sync Width | V Image Size |
  |      20       |      6       |     165      |
       77MHz * 5 = 385MHz overclock (not 417 below)

  800 pixels took 35us, so 56us for 1280, got 60us per line at 20Hz to be safe
  OR: just show 1024x768 images with a small border (128 l/r h; 16 lines t/b v)
  45us per line of 1024 is 27Hz, which may put us more back into "film" framerates

                               60Hz   45Hz       30Hz   20Hz
  1280x800 = 1024000 2.667 417.0MHz  272MHz  209.0MHz
  1280x720 =  921600 2.400
  1024x768 =  786432 2.048 324.0MHz          162.5MHz
   800x480 =  384000 1.000 147.6MHz           75.0MHz

  800x480x30
  30.0Hz *  992 * 496 = 14,760,960  * 5 = 73,804,800 or  74.0MHz or 295MHz cpu / 4
  30.5Hz *  992 * 496 = 15,000,000  * 5 = 75,000,000 or  75.0MHz or 300MHz cpu / 4

  800x480x60
  60.0Hz *  992 * 496 = 29,521,920 * 5 = 147,609,600 or 147.6MHz or 295MHz cpu / 2

  1024x768x60
  60.0Hz * 1344 * 806 = 64,995,840 * 5 = 324,979,200 or 325.0MHz or 325MHz cpu / 1

  1024x768x30
  30.0Hz * 1344 * 806 = 32,497,000 * 5 = 162,489,000 or 162.5MHz or 325MHz cpu / 2
*/

// ------------------------------------------------------------------------------
// some frame timings and defs

/*
const struct dvi_timing __dvi_const(dvi_timing_800x480p_60hz) = {

  .h_front_porch   = 24,
  .h_sync_width    = 72,
  .h_back_porch    = 96,
  .h_active_pixels = 800,

  .v_front_porch   = 3,
  .v_sync_width    = 10,
  .v_back_porch    = 7,
  .v_active_lines  = 480,

  .bit_clk_khz     = 295200
};

const struct dvi_timing __dvi_const(dvi_timing_800x480p_30hz) = {

	.h_front_porch   = 24,
	.h_sync_width    = 72,
	.h_back_porch    = 96,
	.h_active_pixels = 800,

	.v_front_porch   = 3,
	.v_sync_width    = 10,
	.v_back_porch    = 3,
	.v_active_lines  = 480,

	.bit_clk_khz     = 147600
}

# 800x480 29.47 Hz (CVT) hsync: 14.62 kHz; pclk: 14.50 MHz
Modeline "800x480_30.00"   14.50  800 824 896 992  480 483 493 496 -hsync +vsync
                                       24  72  96        3  10   3
*/

#define MODE_H_SYNC_POLARITY   0
#define MODE_H_FRONT_PORCH    24
#define MODE_H_SYNC_WIDTH     72
#define MODE_H_BACK_PORCH     96
#define MODE_H_ACTIVE_PIXELS 800

#define MODE_V_SYNC_POLARITY  0
#define MODE_V_FRONT_PORCH    3
#define MODE_V_SYNC_WIDTH    10
#define MODE_V_BACK_PORCH     7
#define MODE_V_ACTIVE_LINES 480

#define MODE_H_TOTAL_PIXELS ( \
    MODE_H_FRONT_PORCH + MODE_H_SYNC_WIDTH + \
    MODE_H_BACK_PORCH  + MODE_H_ACTIVE_PIXELS \
)

#define MODE_V_TOTAL_LINES  ( \
    MODE_V_FRONT_PORCH + MODE_V_SYNC_WIDTH + \
    MODE_V_BACK_PORCH  + MODE_V_ACTIVE_LINES \
)

#define MODE_V_INACTIVE_LINES  ( \
    MODE_V_FRONT_PORCH + MODE_V_SYNC_WIDTH + \
    MODE_V_BACK_PORCH \
)

#define H_RESOLUTION  (MODE_H_ACTIVE_PIXELS)
#define V_RESOLUTION  (MODE_V_ACTIVE_LINES)

// ------------------------------------------------------------------------------
// DMA channel number allocations

#define DMA_CH_PING 0
#define DMA_CH_PONG 1
#define DMA_CH_READ 2

// ------------------------------------------------------------------------------
// lifecycle calls up from onl to ont

void ont_hx_init();
void ont_hx_frame();
void ont_hx_scanline(uint16_t* buf, uint32_t scan_y);

#endif
