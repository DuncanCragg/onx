#ifndef HSTX_H
#define HSTX_H

// ------------------------------------------------------------------------------
// config for startup.c

#define HSTX_VREG_V      VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED  (300*1000)
#define HSTX_HSTXDIVISOR 4 // 150MHz = 60Hz  100MHz = 40Hz   75MHz = 30Hz

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
#define MODE_V_BACK_PORCH     3
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
