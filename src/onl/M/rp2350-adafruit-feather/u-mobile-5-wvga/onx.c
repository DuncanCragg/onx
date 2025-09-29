
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

#include "hardware/structs/bus_ctrl.h"
#include "hardware/structs/hstx_ctrl.h"
#include "hardware/structs/hstx_fifo.h"
#include "hardware/structs/xip_ctrl.h"
#include "hardware/structs/sio.h"
#include "hardware/xip_cache.h"

#include "hardware/timer.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include <general-defs.h>

#include "tusb.h"

#include "sfe_psram.h"

// ----------------------------------------------------------------------------

#define RP2350_ADAFRUIT_FEATHER

#if defined(RP2350_ADAFRUIT_FEATHER)
#define PSRAM_CS 8
#elif defined(RP2350_ADAFRUIT_FRUIT_JAM)
#define PSRAM_CS 47
#endif

#define DO_TEST_PSRAM // DO_TEST_PSRAM

// ----------------------------------------------------------------------------

#define clockspeed (300*1000)

#define hstxdivisor 4  // 150MHz = 60Hz  100MHz = 40Hz   75MHz = 30Hz

// ----------------------------------------------------------------------------

void set_up_clocks(){

    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(clockspeed, false);

    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        clockspeed * 1000,
        clockspeed * 1000
    );

    clock_configure(
        clk_hstx,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        clockspeed * 1000,
        clockspeed / hstxdivisor  * 1000
    );
}

void set_up_usb(bool wait){
    stdio_init_all();
    if(wait) while (!tud_cdc_connected()) sleep_ms(100);
}

void test_psram(){

    volatile uint8_t* psram_raw_start = (uint8_t*)0x15000000;
    volatile uint8_t* psram_cch_start = (uint8_t*)0x11000000;

    int64_t t_start = time_us_64();
    int full_screen = (V_RESOLUTION * H_RESOLUTION * 2);
    for(size_t i = 0; i < full_screen; i++) {
      psram_raw_start[i] = i & 0xff;
    }
    int64_t t_end = time_us_64();
    double millis  = ((double)(t_end - t_start)) / 1e3;
    printf("PRAM full buffer write %zu bytes: time=%.6fms\n", full_screen, millis);

    #define PSEUDO_RAM_TEST_SIZE (2*1024*1024)
    test_ram_bw("PSRAM", psram_cch_start, PSEUDO_RAM_TEST_SIZE);

    #define STATIC_RAM_TEST_SIZE (100*1024)
    static volatile uint8_t static_ram[STATIC_RAM_TEST_SIZE];
    test_ram_bw("SRAM", static_ram, STATIC_RAM_TEST_SIZE);
}

void set_up_psram(){

    sfe_setup_psram(PSRAM_CS);

#ifdef DO_TEST_PSRAM
    test_psram();
#endif
}

// ----------------------------------------------------------------------------

#define TMDS_CTRL_00 0x354u
#define TMDS_CTRL_01 0x0abu
#define TMDS_CTRL_10 0x154u
#define TMDS_CTRL_11 0x2abu

#define SYNC_V0_H0 (TMDS_CTRL_00 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define SYNC_V0_H1 (TMDS_CTRL_01 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define SYNC_V1_H0 (TMDS_CTRL_10 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define SYNC_V1_H1 (TMDS_CTRL_11 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))

#define HSTX_CMD_RAW         (0x0u << 12)
#define HSTX_CMD_RAW_REPEAT  (0x1u << 12)
#define HSTX_CMD_TMDS        (0x2u << 12)
#define HSTX_CMD_TMDS_REPEAT (0x3u << 12)
#define HSTX_CMD_NOP         (0xfu << 12)

static uint32_t vblank_line_vsync_off[] = {
    HSTX_CMD_RAW_REPEAT | MODE_H_FRONT_PORCH,
    SYNC_V1_H1,
    HSTX_CMD_RAW_REPEAT | MODE_H_SYNC_WIDTH,
    SYNC_V1_H0,
    HSTX_CMD_RAW_REPEAT | (MODE_H_BACK_PORCH + MODE_H_ACTIVE_PIXELS),
    SYNC_V1_H1,
    HSTX_CMD_NOP
};

static uint32_t vblank_line_vsync_on[] = {
    HSTX_CMD_RAW_REPEAT | MODE_H_FRONT_PORCH,
    SYNC_V0_H1,
    HSTX_CMD_RAW_REPEAT | MODE_H_SYNC_WIDTH,
    SYNC_V0_H0,
    HSTX_CMD_RAW_REPEAT | (MODE_H_BACK_PORCH + MODE_H_ACTIVE_PIXELS),
    SYNC_V0_H1,
    HSTX_CMD_NOP
};

static uint32_t vactive_line[] = {
    HSTX_CMD_RAW_REPEAT | MODE_H_FRONT_PORCH,
    SYNC_V1_H1,
    HSTX_CMD_NOP,
    HSTX_CMD_RAW_REPEAT | MODE_H_SYNC_WIDTH,
    SYNC_V1_H0,
    HSTX_CMD_NOP,
    HSTX_CMD_RAW_REPEAT | MODE_H_BACK_PORCH,
    SYNC_V1_H1,
    HSTX_CMD_TMDS       | MODE_H_ACTIVE_PIXELS
};

// ----------------------------------------------------------------------------

static uint16_t ALIGNED linebuf_a[H_RESOLUTION];
static uint16_t ALIGNED linebuf_b[H_RESOLUTION];

static volatile bool    linebuf_ab = true;

static volatile bool    fill_signal = false;
static volatile bool    in_frame = false;
static volatile bool    do_flip = false;
static volatile uint    v_scanline = 2;
static volatile int64_t sign_time = 0;

static bool dma_pong = false;
static bool vactive_cmdlist_posted = false;

void X dma_irq_handler() {

    uint ch_num = dma_pong ? DMA_CH_PONG : DMA_CH_PING;
    dma_channel_hw_t *ch = &dma_hw->ch[ch_num];
    dma_hw->intr = 1u << ch_num;
    dma_pong = !dma_pong;

    if (v_scanline >= MODE_V_FRONT_PORCH && v_scanline < (MODE_V_FRONT_PORCH + MODE_V_SYNC_WIDTH)) {
        ch->read_addr = (uintptr_t)vblank_line_vsync_on;
        ch->transfer_count = count_of(vblank_line_vsync_on);
    } else if (v_scanline < MODE_V_INACTIVE_LINES) {
        ch->read_addr = (uintptr_t)vblank_line_vsync_off;
        ch->transfer_count = count_of(vblank_line_vsync_off);
    } else if (!vactive_cmdlist_posted) {
        ch->read_addr = (uintptr_t)vactive_line;
        ch->transfer_count = count_of(vactive_line);
        vactive_cmdlist_posted = true;
    } else {
        ch->read_addr = (uintptr_t)(linebuf_ab? linebuf_a: linebuf_b);
        ch->transfer_count = MODE_H_ACTIVE_PIXELS/2;
        linebuf_ab = !linebuf_ab;
        fill_signal=true;
        sign_time = time_us_64();
        vactive_cmdlist_posted = false;
    }
    if (!vactive_cmdlist_posted) {
        v_scanline = (v_scanline + 1) % MODE_V_TOTAL_LINES;
        in_frame = v_scanline >= MODE_V_INACTIVE_LINES;
        if(v_scanline==0){
          static int flip_count=0;
          flip_count++;
          if(do_flip){
            do_flip=false;
            if(flip_count>1) printf("flips since last: %d\n", flip_count);
            flip_count=0;
          }
        }
    }
}

/*
    // --------------------------------------------------------------------
    // Configure HSTX's TMDS encoder for RGB332
    hstx_ctrl_hw->expand_tmds =
         2 << HSTX_CTRL_EXPAND_TMDS_L2_NBITS_LSB |
         2 << HSTX_CTRL_EXPAND_TMDS_L1_NBITS_LSB |
         1 << HSTX_CTRL_EXPAND_TMDS_L0_NBITS_LSB |
         0 << HSTX_CTRL_EXPAND_TMDS_L2_ROT_LSB   |
        29 << HSTX_CTRL_EXPAND_TMDS_L1_ROT_LSB   |
        26 << HSTX_CTRL_EXPAND_TMDS_L0_ROT_LSB   ;

    // Pixels (TMDS) come in 4 8-bit chunks. Control symbols (RAW) are an
    // entire 32-bit word.
    hstx_ctrl_hw->expand_shift =
         4 << HSTX_CTRL_EXPAND_SHIFT_ENC_N_SHIFTS_LSB |
         8 << HSTX_CTRL_EXPAND_SHIFT_ENC_SHIFT_LSB    |
         1 << HSTX_CTRL_EXPAND_SHIFT_RAW_N_SHIFTS_LSB |
         0 << HSTX_CTRL_EXPAND_SHIFT_RAW_SHIFT_LSB;

    // --------------------------------------------------------------------
    // Configure HSTX's TMDS encoder for RGB565
    hstx_ctrl_hw->expand_tmds =
         4 << HSTX_CTRL_EXPAND_TMDS_L2_NBITS_LSB |
         5 << HSTX_CTRL_EXPAND_TMDS_L1_NBITS_LSB |
         4 << HSTX_CTRL_EXPAND_TMDS_L0_NBITS_LSB |
         8 << HSTX_CTRL_EXPAND_TMDS_L2_ROT_LSB   |
         3 << HSTX_CTRL_EXPAND_TMDS_L1_ROT_LSB   |
        29 << HSTX_CTRL_EXPAND_TMDS_L0_ROT_LSB   ;

    // Pixels (TMDS) come in 2 16-bit chunks. Control symbols (RAW) are an
    // entire 32-bit word.
    hstx_ctrl_hw->expand_shift =
         2 << HSTX_CTRL_EXPAND_SHIFT_ENC_N_SHIFTS_LSB |
        16 << HSTX_CTRL_EXPAND_SHIFT_ENC_SHIFT_LSB    |
         1 << HSTX_CTRL_EXPAND_SHIFT_RAW_N_SHIFTS_LSB |
         0 << HSTX_CTRL_EXPAND_SHIFT_RAW_SHIFT_LSB    ;

    // --------------------------------------------------------------------
    // Configure HSTX's TMDS encoder for RGB888
    hstx_ctrl_hw->expand_tmds =
         7 << HSTX_CTRL_EXPAND_TMDS_L2_NBITS_LSB |
         7 << HSTX_CTRL_EXPAND_TMDS_L1_NBITS_LSB |
         7 << HSTX_CTRL_EXPAND_TMDS_L0_NBITS_LSB |
        16 << HSTX_CTRL_EXPAND_TMDS_L2_ROT_LSB   |
         8 << HSTX_CTRL_EXPAND_TMDS_L1_ROT_LSB   |
         0 << HSTX_CTRL_EXPAND_TMDS_L0_ROT_LSB   ;

    // Pixels and control symbols (RAW) are an
    // entire 32-bit word.
    hstx_ctrl_hw->expand_shift =
         1 << HSTX_CTRL_EXPAND_SHIFT_ENC_N_SHIFTS_LSB |
         0 << HSTX_CTRL_EXPAND_SHIFT_ENC_SHIFT_LSB    |
         1 << HSTX_CTRL_EXPAND_SHIFT_RAW_N_SHIFTS_LSB |
         0 << HSTX_CTRL_EXPAND_SHIFT_RAW_SHIFT_LSB    ;
*/

void set_up_dma(){

    // --------------------------------------------------------------------
    // Configure HSTX's TMDS encoder for RGB555
    hstx_ctrl_hw->expand_tmds =
         4 << HSTX_CTRL_EXPAND_TMDS_L2_NBITS_LSB |
         4 << HSTX_CTRL_EXPAND_TMDS_L1_NBITS_LSB |
         4 << HSTX_CTRL_EXPAND_TMDS_L0_NBITS_LSB |
         7 << HSTX_CTRL_EXPAND_TMDS_L2_ROT_LSB   |
         2 << HSTX_CTRL_EXPAND_TMDS_L1_ROT_LSB   |
        29 << HSTX_CTRL_EXPAND_TMDS_L0_ROT_LSB   ;

    // Pixels (TMDS) come in 2 16-bit chunks. Control symbols (RAW) are an
    // entire 32-bit word.
    hstx_ctrl_hw->expand_shift =
         2 << HSTX_CTRL_EXPAND_SHIFT_ENC_N_SHIFTS_LSB |
        16 << HSTX_CTRL_EXPAND_SHIFT_ENC_SHIFT_LSB    |
         1 << HSTX_CTRL_EXPAND_SHIFT_RAW_N_SHIFTS_LSB |
         0 << HSTX_CTRL_EXPAND_SHIFT_RAW_SHIFT_LSB    ;

    // Serial output config: clock period of 5 cycles, pop from command
    // expander every 5 cycles, shift the output shiftreg by 2 every cycle.
    hstx_ctrl_hw->csr = 0;
    hstx_ctrl_hw->csr =
              HSTX_CTRL_CSR_EXPAND_EN_BITS |
        5u << HSTX_CTRL_CSR_CLKDIV_LSB     |
        5u << HSTX_CTRL_CSR_N_SHIFTS_LSB   |
        2u << HSTX_CTRL_CSR_SHIFT_LSB      |
              HSTX_CTRL_CSR_EN_BITS        ;

    // --------------------------------------------------------------------
    // GPIO12->GPIO19 is HSTX 0->7
    //
    // Feather:
    //
    //   GP14 CK+  GP15 CK-  12+2,3
    //   GP18 D0+  GP19 D0-  12+6,7
    //   GP16 D1+  GP17 D1-  12+4,5
    //   GP12 D2+  GP13 D2-  12+0,1
    //
    // Fruit Jam:
    //
    //   GP12 CK+  GP13 CK-  12+0,1
    //   GP14 D0+  GP15 D0-  12+2,3
    //   GP16 D1+  GP17 D1-  12+4,5
    //   GP18 D2+  GP19 D2-  12+6,7
    //
#if defined(RP2350_ADAFRUIT_FEATHER)
    hstx_ctrl_hw->bit[2] = HSTX_CTRL_BIT0_CLK_BITS;
    hstx_ctrl_hw->bit[3] = HSTX_CTRL_BIT0_CLK_BITS | HSTX_CTRL_BIT0_INV_BITS;
    static const int lane_to_output_bit[3] = {6,4,0};
#elif defined(RP2350_ADAFRUIT_FRUIT_JAM)
    hstx_ctrl_hw->bit[0] = HSTX_CTRL_BIT0_CLK_BITS;
    hstx_ctrl_hw->bit[1] = HSTX_CTRL_BIT0_CLK_BITS | HSTX_CTRL_BIT0_INV_BITS;
    static const int lane_to_output_bit[3] = {2,4,6};
#endif
    for (uint lane = 0; lane < 3; ++lane) {
        int bit = lane_to_output_bit[lane];
        uint32_t lane_data_sel_bits = (lane * 10    ) << HSTX_CTRL_BIT0_SEL_P_LSB |
                                      (lane * 10 + 1) << HSTX_CTRL_BIT0_SEL_N_LSB;
#if defined(RP2350_ADAFRUIT_FEATHER)
        hstx_ctrl_hw->bit[bit    ] = lane_data_sel_bits;
        hstx_ctrl_hw->bit[bit + 1] = lane_data_sel_bits | HSTX_CTRL_BIT0_INV_BITS;
#elif defined(RP2350_ADAFRUIT_FRUIT_JAM)
        hstx_ctrl_hw->bit[bit    ] = lane_data_sel_bits | HSTX_CTRL_BIT0_INV_BITS;
        hstx_ctrl_hw->bit[bit + 1] = lane_data_sel_bits;
#endif
    }

    for (int i = 12; i <= 19; ++i) {
        gpio_set_function(i, 0);
    }

    dma_channel_config c;

    c = dma_channel_get_default_config(DMA_CH_PING);
    channel_config_set_chain_to(&c, DMA_CH_PONG);
    channel_config_set_dreq(&c, DREQ_HSTX);
//  channel_config_set_high_priority(&c, true);
    dma_channel_configure(
        DMA_CH_PING,
        &c,
        &hstx_fifo_hw->fifo,
        vblank_line_vsync_off,
        count_of(vblank_line_vsync_off),
        false
    );

    c = dma_channel_get_default_config(DMA_CH_PONG);
    channel_config_set_chain_to(&c, DMA_CH_PING);
    channel_config_set_dreq(&c, DREQ_HSTX);
//  channel_config_set_high_priority(&c, true);
    dma_channel_configure(
        DMA_CH_PONG,
        &c,
        &hstx_fifo_hw->fifo,
        vblank_line_vsync_off,
        count_of(vblank_line_vsync_off),
        false
    );

    dma_hw->ints0 = (1u << DMA_CH_PING) | (1u << DMA_CH_PONG);
    dma_hw->inte0 = (1u << DMA_CH_PING) | (1u << DMA_CH_PONG);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;

    dma_channel_start(DMA_CH_PING);
}

// ------------------------------------------------------

#define  CORE1_STACK_SIZE 512
uint32_t core1_stack[CORE1_STACK_SIZE];

static void __not_in_flash_func(core0_main)();
static void __not_in_flash_func(core1_main)();

void __not_in_flash_func(core0_main)() {

    set_up_clocks();

    set_up_usb(false);

    printf("\n------------ core 0 start ---------------------\n");

    set_up_psram();

    ont_hx_init();

    multicore_launch_core1_with_stack(core1_main, core1_stack, CORE1_STACK_SIZE);

    while(1){

        while(do_flip) tight_loop_contents();

        ont_hx_frame();

        do_flip=true;
    }
}

#define SCANLINE_TIMER_BEGIN                    \
        int64_t sign_time_x = sign_time;        \
        bool    linebuf_abx = linebuf_ab;

#define SCANLINE_TIMER_END                      \
        static int f=0; f++;                    \
        if((f % (20 * 500) == 0)){              \
          int64_t done_time_x = time_us_64();   \
          printf("signal->done %.3lldus; %s\n", \
                  done_time_x-sign_time_x,      \
                  linebuf_abx==linebuf_ab?      \
                     "same line": "diff line"   \
          );                                    \
        }

void __not_in_flash_func(core1_main)(){

    printf("\n------------ core 1 start ---------------------\n");

    set_up_dma();

    while(1){

        while(!fill_signal) tight_loop_contents();

        int scan_y = v_scanline - MODE_V_INACTIVE_LINES;

    ;   if(scan_y < 0 || scan_y > 478) continue; // why not 479?

        fill_signal = false;

        SCANLINE_TIMER_BEGIN

        uint16_t* buf = (linebuf_ab? linebuf_a: linebuf_b);

        ont_hx_scanline(buf, scan_y);

        SCANLINE_TIMER_END
    }
}

int __not_in_flash_func(main)() {
  core0_main();
}

















