#ifndef HSTX_H
#define HSTX_H

// ------------------------------------------------------------------------------

#define WS10

#ifdef WS10
#define HSTX_VREG_V         VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ (385*1000)
#define HSTX_HSTXDIVISOR    1
#define H_PLUS_V_PLUS
// 385/1=385 /5=77MHz pxclk *1280x800 = 60Hz
#endif



#ifdef WS10
#define MODE_H_SYNC_POLARITY    1
#define MODE_H_FRONT_PORCH    160
#define MODE_H_SYNC_WIDTH      32
#define MODE_H_BACK_PORCH      40
#define MODE_H_ACTIVE_PIXELS 1280

#define MODE_V_SYNC_POLARITY    1
#define MODE_V_FRONT_PORCH     20
#define MODE_V_SYNC_WIDTH       6
#define MODE_V_BACK_PORCH      30
#define MODE_V_ACTIVE_LINES   800
#endif
#ifdef TOSH
#define MODE_H_SYNC_POLARITY    1
#define MODE_H_FRONT_PORCH     48
#define MODE_H_SYNC_WIDTH      32
#define MODE_H_BACK_PORCH      80
#define MODE_H_ACTIVE_PIXELS 1280

#define MODE_V_SYNC_POLARITY    1
#define MODE_V_FRONT_PORCH      3
#define MODE_V_SYNC_WIDTH       6
#define MODE_V_BACK_PORCH      14
#define MODE_V_ACTIVE_LINES   800
#endif

// ------------------------------------------------------------------------------

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
