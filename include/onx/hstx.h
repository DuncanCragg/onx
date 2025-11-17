#ifndef HSTX_H
#define HSTX_H

// ------------------------------------------------------------------------------

#define PCMON

#ifdef WS10
#define WS10_PUSH_PSRAM
#ifdef WS10_PUSH_PSRAM
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (356*1000)
#define HSTX_CLOCK_DIVIDER         1
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   2 // 178MHz !!
// 356/1=356 /5=71MHz pxclk *1280x800 = 60Hz
#else
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (384*1000)
#define HSTX_CLOCK_DIVIDER         1
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   3
// 384/1=384 /5=77MHz pxclk *1280x800 = 60Hz
#endif
#endif

#ifdef PCMON
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (384*1000)
#define HSTX_CLOCK_DIVIDER         1
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   3
#endif

#ifdef TOSH
#ifdef TOSH_24HZ
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (372*1000)
#define HSTX_CLOCK_DIVIDER         1
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   3
//    372 /5=74.4MHz *1920x1080 = 24Hz
#else
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (356*1000)
#define HSTX_CLOCK_DIVIDER         1
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   2 // 178MHz !!
// 356/1=356 /5=71MHz pxclk *1280x800 = 60Hz
#endif
#endif

#ifdef WS5
#define WS5_40HZ
#ifdef WS5_40HZ
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (384*1000)
#define HSTX_CLOCK_DIVIDER         4
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   3
// 384/4=96 /5=19.2MHz pxclk *800x480 = 39.4Hz
#else // 60Hz
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (300*1000)
#define HSTX_CLOCK_DIVIDER         2
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   2
// 300/2=150 /5=30MHz pxclk *800x480 = 60Hz
#endif
#endif

#ifdef ELECROW5
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (300*1000)
#define HSTX_CLOCK_DIVIDER         2
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   2
// 300/2=150 /5=30MHz pxclk *800x480 = 60Hz
#endif

#ifdef WS10

#define V_PLUS_H_PLUS

#ifdef WS10_PUSH_PSRAM

#define MODE_H_FRONT_PORCH     30
#define MODE_H_SYNC_WIDTH      32
#define MODE_H_BACK_PORCH      30
#define MODE_H_ACTIVE_PIXELS 1280

#else

#define MODE_H_FRONT_PORCH    160
#define MODE_H_SYNC_WIDTH      32
#define MODE_H_BACK_PORCH      40
#define MODE_H_ACTIVE_PIXELS 1280

#endif

#define MODE_V_FRONT_PORCH     20
#define MODE_V_SYNC_WIDTH       6
#define MODE_V_BACK_PORCH      30
#define MODE_V_ACTIVE_LINES   800

#endif

#ifdef WS5

#define V_MINUS_H_MINUS

#define MODE_H_FRONT_PORCH     40
#define MODE_H_SYNC_WIDTH      48
#define MODE_H_BACK_PORCH      40
#define MODE_H_ACTIVE_PIXELS  800

#define MODE_V_FRONT_PORCH     13
#define MODE_V_SYNC_WIDTH       3
#define MODE_V_BACK_PORCH      29
#define MODE_V_ACTIVE_LINES   480
#endif

#ifdef ELECROW5

#define V_PLUS_H_MINUS

#define MODE_H_FRONT_PORCH     16
#define MODE_H_SYNC_WIDTH      80
#define MODE_H_BACK_PORCH      96
#define MODE_H_ACTIVE_PIXELS  800

#define MODE_V_FRONT_PORCH      1
#define MODE_V_SYNC_WIDTH       3
#define MODE_V_BACK_PORCH      13
#define MODE_V_ACTIVE_LINES   480
#endif

#ifdef PCMON

#define V_PLUS_H_MINUS

#define MODE_H_FRONT_PORCH     72
#define MODE_H_SYNC_WIDTH     128
#define MODE_H_BACK_PORCH     200
#define MODE_H_ACTIVE_PIXELS 1280

#define MODE_V_FRONT_PORCH      3
#define MODE_V_SYNC_WIDTH       6
#define MODE_V_BACK_PORCH      22
#define MODE_V_ACTIVE_LINES   720

#endif


#ifdef TOSH

#ifdef TOSH_24HZ

#define V_PLUS_H_PLUS

#define MODE_H_FRONT_PORCH    638
#define MODE_H_SYNC_WIDTH      44
#define MODE_H_BACK_PORCH     148
#define MODE_H_ACTIVE_PIXELS 1920

#define MODE_V_FRONT_PORCH      4
#define MODE_V_SYNC_WIDTH       5
#define MODE_V_BACK_PORCH      36
#define MODE_V_ACTIVE_LINES  1080

#else

#define V_MINUS_H_PLUS

#define MODE_H_FRONT_PORCH     48
#define MODE_H_SYNC_WIDTH      32
#define MODE_H_BACK_PORCH      80
#define MODE_H_ACTIVE_PIXELS 1280

#define MODE_V_FRONT_PORCH      3
#define MODE_V_SYNC_WIDTH       6
#define MODE_V_BACK_PORCH      14
#define MODE_V_ACTIVE_LINES   800

#endif

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

#define LINEBUF_LINES 16

// ------------------------------------------------------------------------------
// DMA channel number allocations

#define DMA_CH_PING 0
#define DMA_CH_PONG 1
#define DMA_CH_READ 2

// ------------------------------------------------------------------------------
// lifecycle calls up from onl to ont

void ont_hx_init();
void ont_hx_frame();
void ont_hx_scanline(uint16_t* buf, uint16_t scan_y);

#endif
