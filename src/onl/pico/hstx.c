
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/time.h>

#include <hardware/structs/bus_ctrl.h>
#include <hardware/structs/hstx_ctrl.h>
#include <hardware/structs/hstx_fifo.h>
#include <hardware/structs/xip_ctrl.h>
#include <hardware/structs/sio.h>
#include <hardware/xip_cache.h>
#include <hardware/timer.h>
#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>

#include <sync-and-mem.h>

#include <onx/startup.h>
#include <onx/log.h>
#include <onx/time.h>
#include <onx/hstx.h>

// --------------------------------------
// config for startup.c

const uint8_t  startup_vreg_v              = HSTX_VREG_V;
const uint32_t startup_clockspeed_khz      = HSTX_CLOCKSPEED_KHZ;
const uint8_t  startup_hstx_clock_divider  = HSTX_CLOCK_DIVIDER;
const uint8_t  startup_flash_clock_divider = HSTX_FLASH_CLOCK_DIVIDER;
const uint8_t  psram_clock_divider         = HSTX_PSRAM_CLOCK_DIVIDER;

// ----------------------------------------------------------------------------

/*
DVI spec:

      C1|C0=V|H
       0|0 0010101011 0x00ab
       0|1 1101010100 0x0354 \__ what order? C0|C1?
       1|0 0010101010 0x00aa /
       1|1 1101010101 0x0355

 ctl0 ctl1 ctl2 ctl3 Data Period Type
   1    0    0    0  Video Data Period - what code for 0/1?

 B: 0x02cc (10 1100 1100)
 G: 0x0133 (01 0011 0011)
 R: 0x02cc (10 1100 1100)

*/
#define DVI_00 0x0354 // 0x00ab <---these are from pico-examples
#define DVI_01 0x0154 // 0x00aa \__ what order?
#define DVI_10 0x00ab // 0x0354 /
#define DVI_11 0x02ab // 0x0355

#define CTLB DVI_00 // 0x02cc - what should be here?
#define CTLG DVI_10 // 0x0133
#define CTLR DVI_00 // 0x02cc

#ifdef V_MINUS_H_MINUS
//                  B (sync)    G (ctl0/1)     R (ctl2/3)
#define SYNC_V_N_H_N (DVI_11 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_N_H_Y (DVI_10 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_N (DVI_01 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_Y (DVI_00 | (CTLG << 10) | (CTLR << 20))
#endif

#ifdef V_MINUS_H_PLUS // I know it makes no sense but I don't understand enough (Tosh)
//                  B (sync)  G (ctl0/1)       R (ctl2/3)
#define SYNC_V_N_H_N (DVI_01 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_N_H_Y (DVI_11 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_N (DVI_00 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_Y (DVI_10 | (CTLG << 10) | (CTLR << 20))
#endif

#ifdef V_PLUS_H_MINUS
//                  B (sync)  G (ctl0/1)       R (ctl2/3)
#define SYNC_V_N_H_N (DVI_01 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_N_H_Y (DVI_00 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_N (DVI_11 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_Y (DVI_10 | (CTLG << 10) | (CTLR << 20))
#endif

#ifdef V_PLUS_H_PLUS
//                  B (sync)  G (ctl0/1)       R (ctl2/3)
#define SYNC_V_N_H_N (DVI_00 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_N_H_Y (DVI_01 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_N (DVI_10 | (CTLG << 10) | (CTLR << 20))
#define SYNC_V_Y_H_Y (DVI_11 | (CTLG << 10) | (CTLR << 20))
#endif

#define HSTX_CMD_RAW         (0x0u << 12)
#define HSTX_CMD_RAW_REPEAT  (0x1u << 12)
#define HSTX_CMD_TMDS        (0x2u << 12)
#define HSTX_CMD_TMDS_REPEAT (0x3u << 12)
#define HSTX_CMD_NOP         (0xfu << 12)

static uint32_t vblank_line_vsync_off[] = {
    HSTX_CMD_RAW_REPEAT | MODE_H_FRONT_PORCH,
    SYNC_V_N_H_N,
    HSTX_CMD_RAW_REPEAT | MODE_H_SYNC_WIDTH,
    SYNC_V_N_H_Y,
    HSTX_CMD_RAW_REPEAT | (MODE_H_BACK_PORCH + MODE_H_ACTIVE_PIXELS),
    SYNC_V_N_H_N,
    HSTX_CMD_NOP
};

static uint32_t vblank_line_vsync_on[] = {
    HSTX_CMD_RAW_REPEAT | MODE_H_FRONT_PORCH,
    SYNC_V_Y_H_N,
    HSTX_CMD_RAW_REPEAT | MODE_H_SYNC_WIDTH,
    SYNC_V_Y_H_Y,
    HSTX_CMD_RAW_REPEAT | (MODE_H_BACK_PORCH + MODE_H_ACTIVE_PIXELS),
    SYNC_V_Y_H_N,
    HSTX_CMD_NOP
};

static uint32_t vactive_line[] = {
    HSTX_CMD_RAW_REPEAT | MODE_H_FRONT_PORCH,
    SYNC_V_N_H_N,
    HSTX_CMD_NOP,
    HSTX_CMD_RAW_REPEAT | MODE_H_SYNC_WIDTH,
    SYNC_V_N_H_Y,
    HSTX_CMD_NOP,
    HSTX_CMD_RAW_REPEAT | MODE_H_BACK_PORCH,
    SYNC_V_N_H_N,
    HSTX_CMD_TMDS       | MODE_H_ACTIVE_PIXELS
};

// ----------------------------------------------------------------------------

static uint16_t ALIGNED  linebuf_a[LINEBUF_LINES][H_RESOLUTION];
static uint16_t ALIGNED  linebuf_b[LINEBUF_LINES][H_RESOLUTION];
static volatile bool     linebuf_ab = true;
static volatile uint16_t linebuf_line=0;
static volatile bool     linebuf_switched = false;
static volatile uint16_t linebuf_scanline = 0;

static volatile bool     new_frame = false;

static volatile uint16_t v_scanline = 2;

static bool dma_pong = false;
static bool vactive_cmdlist_posted = false; // h blank/sync period

static void __not_in_flash_func(dma_irq_handler)() {

    uint ch_num = dma_pong ? DMA_CH_PONG : DMA_CH_PING;
    dma_channel_hw_t *ch = &dma_hw->ch[ch_num];
    dma_hw->intr = 1u << ch_num;
    dma_pong = !dma_pong;

    if (v_scanline >= MODE_V_FRONT_PORCH &&
        v_scanline < (MODE_V_FRONT_PORCH + MODE_V_SYNC_WIDTH)) { // v-sync region after front porch

      ch->read_addr = (uintptr_t)vblank_line_vsync_on;
      ch->transfer_count = count_of(vblank_line_vsync_on);

    } else if (v_scanline < MODE_V_INACTIVE_LINES) { // front porch, back porch

      ch->read_addr = (uintptr_t)vblank_line_vsync_off;
      ch->transfer_count = count_of(vblank_line_vsync_off);

    } else if (!vactive_cmdlist_posted) { // active lines, h-sync (blank); visible pixels end

      ch->read_addr = (uintptr_t)vactive_line;
      ch->transfer_count = count_of(vactive_line);
      vactive_cmdlist_posted = true;

    } else { // active lines, h-sync (blank) end; visible pixels start

      if(v_scanline==MODE_V_INACTIVE_LINES){
        linebuf_line=0;
        linebuf_ab = false;
        linebuf_switched=true;
        linebuf_scanline=LINEBUF_LINES;
      }
      else
      if(linebuf_line == LINEBUF_LINES){
        linebuf_line=0;
        linebuf_ab = !linebuf_ab;
        linebuf_switched=true;
        linebuf_scanline=v_scanline - MODE_V_INACTIVE_LINES + LINEBUF_LINES;
      }
      ch->read_addr = (uintptr_t)(linebuf_ab? linebuf_a[linebuf_line]: linebuf_b[linebuf_line]);
      ch->transfer_count = MODE_H_ACTIVE_PIXELS/2;
      linebuf_line++;
      vactive_cmdlist_posted = false;
    }
    if (!vactive_cmdlist_posted) { // i.e., not the partial-line h sync/blank bit
      v_scanline = (v_scanline + 1) % MODE_V_TOTAL_LINES;
      if(v_scanline==0){
        linebuf_line=0;
        linebuf_ab = true; // REVISIT: couldn't this trash the final line?
        linebuf_switched=true;
        linebuf_scanline=0;
        new_frame=true; // REVISIT: set pending to set true on re-entry, since there's a final line going out?
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

static void set_up_hstx(){

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
    //   GP14 CK+  GP15 CK-  12+2,3 14
    //   GP18 D0+  GP19 D0-  12+6,7 18
    //   GP16 D1+  GP17 D1-  12+4,5 16
    //   GP12 D2+  GP13 D2-  12+0,1 12
    //
    // Fruit Jam:
    //
    //   GP12 CK+  GP13 CK-  12+0,1 12
    //   GP14 D0+  GP15 D0-  12+2,3 14
    //   GP16 D1+  GP17 D1-  12+4,5 16
    //   GP18 D2+  GP19 D2-  12+6,7 18
    //
    // REVISIT: if(PICO_DEFAULT_HSTX_INV) for clock too?
    hstx_ctrl_hw->bit[PICO_DEFAULT_HSTX_CK_PIN-12] = HSTX_CTRL_BIT0_CLK_BITS;
    hstx_ctrl_hw->bit[PICO_DEFAULT_HSTX_CK_PIN-11] = HSTX_CTRL_BIT0_CLK_BITS | HSTX_CTRL_BIT0_INV_BITS;
    static const int lane_to_output_bit[3] = {
      PICO_DEFAULT_HSTX_D0_PIN-12,
      PICO_DEFAULT_HSTX_D1_PIN-12,
      PICO_DEFAULT_HSTX_D2_PIN-12
    };
    for (uint lane = 0; lane < 3; ++lane) {
        int bit = lane_to_output_bit[lane];
        uint32_t lane_data_sel_bits = (lane * 10    ) << HSTX_CTRL_BIT0_SEL_P_LSB |
                                      (lane * 10 + 1) << HSTX_CTRL_BIT0_SEL_N_LSB;
        if(PICO_DEFAULT_HSTX_INV){
            hstx_ctrl_hw->bit[bit    ] = lane_data_sel_bits;
            hstx_ctrl_hw->bit[bit + 1] = lane_data_sel_bits | HSTX_CTRL_BIT0_INV_BITS;
        } else {
            hstx_ctrl_hw->bit[bit    ] = lane_data_sel_bits | HSTX_CTRL_BIT0_INV_BITS;
            hstx_ctrl_hw->bit[bit + 1] = lane_data_sel_bits;
        }
    }
    for (int i = 12; i <= 19; ++i) {
        gpio_set_function(i, 0); // REVISIT: use gpio api
    }
}

static void set_up_dma(){

  dma_channel_config c;

  c = dma_channel_get_default_config(DMA_CH_PING);
  channel_config_set_chain_to(&c, DMA_CH_PONG);
  channel_config_set_dreq(&c, DREQ_HSTX);
  channel_config_set_high_priority(&c, true);
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
  channel_config_set_high_priority(&c, true);
  dma_channel_configure(
      DMA_CH_PONG,
      &c,
      &hstx_fifo_hw->fifo,
      vblank_line_vsync_off,
      count_of(vblank_line_vsync_off),
      false
  );
  irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
  bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;
}

static void start_dma(){
  dma_hw->ints0 = (1u << DMA_CH_PING) | (1u << DMA_CH_PONG);
  dma_hw->inte0 = (1u << DMA_CH_PING) | (1u << DMA_CH_PONG);
  irq_set_enabled(DMA_IRQ_0, true);
  dma_channel_start(DMA_CH_PING);
  log_write("DMA started\n");
}

// ------------------------------------------------------

void startup_core0_init(){

  ont_hx_init();
}

void __not_in_flash_func(startup_core0_loop)(){
  do{
    bool nf=new_frame;
    ont_hx_frame(nf);
  ; if(!nf && new_frame) continue;
    new_frame=false;
  ; break;
  } while(1);
}

#define DO_SCANLINE_TIMER
#ifdef  DO_SCANLINE_TIMER
#define SCANLINE_TIMER_BEGIN                                                 \
        static uint32_t scanline_skips=0;                                    \
        static uint16_t low_line_at_skip = 65535;                            \
        int64_t start_time=time_us();                                        \

#define SCANLINE_TIMER_MID                                                   \
        if(linebuf_scanline != linebuf_scanline_at_entry){                   \
          scanline_skips++;                                                  \
          if(line<low_line_at_skip) low_line_at_skip=line;                   \
          break;                                                             \
        }                                                                    \

#define SCANLINE_TIMER_END                                                       \
        if(v_scanline>=400){                                                     \
          static int f=0; f++;                                                   \
          if(f % 5000 == 0){                                                     \
            int64_t end_time=time_us();                                          \
            log_write("skips=%u lowest line=%u line time=%.3lluus\n",            \
                       scanline_skips, low_line_at_skip,                         \
                       (end_time-start_time)/LINEBUF_LINES);                     \
            scanline_skips=0; low_line_at_skip=65535;                            \
          }                                                                      \
        }
#else
#define SCANLINE_TIMER_BEGIN
#define SCANLINE_TIMER_MID
#define SCANLINE_TIMER_END
#endif

void startup_core1_init(){
  set_up_hstx();
  set_up_dma();
  start_dma();
  log_write("HSTX and DMA set up --------\n");
}

void __not_in_flash_func(startup_core1_loop)(){

  if(!linebuf_switched) return;
  linebuf_switched = false;

//REVISIT: we're going to have to move this loop into ont_hx
  SCANLINE_TIMER_BEGIN
  uint16_t linebuf_scanline_at_entry=linebuf_scanline;
  for(uint16_t line=0; line < LINEBUF_LINES; line++){
    uint16_t scan_y=linebuf_scanline_at_entry+line;
  ; if(scan_y >= V_RESOLUTION) break;
    uint16_t* buf = (linebuf_ab? linebuf_b[line]: linebuf_a[line]); // REVISIT obvs hacky below
    uint16_t* puf = line==0? (linebuf_ab? linebuf_a[LINEBUF_LINES-1]: linebuf_b[LINEBUF_LINES-1]): 0;
    ont_hx_scanline(buf, puf, scan_y, line==LINEBUF_LINES-1);
    SCANLINE_TIMER_MID
  }
  SCANLINE_TIMER_END
}


















