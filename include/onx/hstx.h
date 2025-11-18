#ifndef HSTX_H
#define HSTX_H

// ------------------------------------------------------------------------------

#define WS10

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
#define TOSH_1024
#ifdef  TOSH_1024
#define HSTX_VREG_V                VREG_VOLTAGE_1_30
#define HSTX_CLOCKSPEED_KHZ       (324*1000)
#define HSTX_CLOCK_DIVIDER         1
#define HSTX_FLASH_CLOCK_DIVIDER   3
#define HSTX_PSRAM_CLOCK_DIVIDER   2 // 162MHz
//    324 /5=64.8MHz *1024x768
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
// #define WS5_40HZ
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

#define V_MINUS_H_PLUS // actually reports -/-: this is specially hacked for Tosh

#ifdef TOSH_1024

#define MODE_H_FRONT_PORCH     24
#define MODE_H_SYNC_WIDTH     136
#define MODE_H_BACK_PORCH     160
#define MODE_H_ACTIVE_PIXELS 1024

#define MODE_V_FRONT_PORCH      3
#define MODE_V_SYNC_WIDTH       6
#define MODE_V_BACK_PORCH      29
#define MODE_V_ACTIVE_LINES   768

#else

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

extern volatile int64_t ont_hx_frame_time;

void ont_hx_init();
void ont_hx_frame();
void ont_hx_scanline(uint16_t* buf, uint16_t* puf, uint16_t scan_y);

#endif
