
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/address_mapped.h>
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/regs/addressmap.h>
#include <hardware/spi.h>
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip_ctrl.h>
#include <hardware/sync.h>

#include <pico/binary_info.h>
#include <pico/flash.h>
#include <pico/time.h>

#include <onx/log.h>
#include <onx/psram.h>

// For this chip: APS6404L-3SQR-ZR
// https://www.mouser.com/ProductDetail/AP-Memory/APS6404L-3SQR-ZR?qs=IS%252B4QmGtzzpDOdsCIglviw%3D%3D
// Code commes from somewhere in here, apparently: https://github.com/raspberrypi/pico-sdk-rp2350/
// Licence at end of file

// --------------------------------------------------------------------

// For PSRAM timing calculations - to use int math, we work in femto seconds (fs) (1e-15),
// NOTE: This idea is from MicroPython work on PSRAM.
#define SFE_SEC_TO_FS 1000000000000000ll

// Max select pulse width = 8us => 8e6 ns => 8000 ns => 8000 * 1e6 fs => 8000e6 fs
// Additionally, the MAX select is in units of 64 clock cycles - will use a constant that
// takes this into account - so 8000e6 fs / 64 = 125e6 fs
const uint32_t SFE_PSRAM_MAX_SELECT_FS64 = 125000000;

// min deselect pulse width = 50ns => 50 * 1e6 fs => 50e7 fs
const uint32_t SFE_PSRAM_MIN_DESELECT_FS = 50000000;

// from psram datasheet - max Freq with VDDat 3.3v - SparkFun RP2350 boards run at 3.3v.
// If VDD = 3.0 Max Freq is 133 Mhz
const uint32_t SFE_PSRAM_MAX_SCK_HZ = 109000000;

// --------------------------------------------------------------------

const uint8_t PSRAM_ID = 0x5D;

const uint8_t PSRAM_CMD_QUAD_END    = 0xf5;
const uint8_t PSRAM_CMD_QUAD_ENABLE = 0x35;
const uint8_t PSRAM_CMD_READ_ID     = 0x9f;
const uint8_t PSRAM_CMD_RSTEN       = 0x66;
const uint8_t PSRAM_CMD_RST         = 0x99;
const uint8_t PSRAM_CMD_QUAD_READ   = 0xeb;
const uint8_t PSRAM_CMD_QUAD_WRITE  = 0x38;
const uint8_t PSRAM_CMD_NOOP        = 0xff;

// --------------------------------------------------------------------

#define STRIDE 64

static bool psram_test_ram_bw(char* region, volatile uint8_t* start, size_t len){

    size_t number_of_test_addresses = len / STRIDE;

    for(size_t i = 0; i < number_of_test_addresses; i++) {
      start[i * STRIDE] = (i & 0xFF);
      if(start[i * STRIDE] != (i & 0xFF)) {
        log_write("RAM mismatch on write in %s at i=%zu %d!=%d\n", region, i, start[i * STRIDE], (i & 0xFF));
;       return false;
      }
    }

    int64_t t_start = time_us();
    for(size_t i = 0; i < number_of_test_addresses; i++) {
      if(start[i * STRIDE] != (i & 0xFF)){
        log_write("RAM mismatch on read in %s at i=%zu %x!=%x %b!=%b\n", region, i, start[i * STRIDE], (i & 0xFF),
                                                                                    start[i * STRIDE], (i & 0xFF));
;       return false;
      }
    }
    int64_t t_end = time_us();

    double millis  = ((double)(t_end - t_start)) / 1e3;
    double seconds = ((double)(t_end - t_start)) / 1e6;
    double mbps = (double)number_of_test_addresses / (1024.0 * 1024.0) / seconds;
    log_write("RAM test in region %s %zu bytes: time=%.6fms, approx %.2fMB/s\n",
                               region, number_of_test_addresses, millis, mbps);

    return true;
}

void psram_tests(){

    volatile uint8_t* psram_raw_start = (uint8_t*)0x15000000;
    volatile uint8_t* psram_cch_start = (uint8_t*)0x11000000;

    int64_t t_start = time_us();
    uint full_screen = (800 * 480 * 2);
    for(size_t i = 0; i < full_screen; i++) {
      psram_raw_start[i] = i & 0xff;
    }
    int64_t t_end = time_us();
    double millis  = ((double)(t_end - t_start)) / 1e3;
    log_write("PRAM full buffer write %zu bytes: time=%.6fms\n", full_screen, millis);

    #define PSEUDO_RAM_TEST_SIZE (2*1024*1024)
    psram_test_ram_bw("PSRAM", psram_cch_start, PSEUDO_RAM_TEST_SIZE);

    #define STATIC_RAM_TEST_SIZE (100*1024)
    static volatile uint8_t static_ram[STATIC_RAM_TEST_SIZE];
    psram_test_ram_bw("SRAM", static_ram, STATIC_RAM_TEST_SIZE);
}

// --------------------------------------------------------------------

static size_t __no_inline_not_in_flash_func(get_psram_size)(void) {

    size_t psram_size = 0;
    uint32_t intr_stash = save_and_disable_interrupts();

    qmi_hw->direct_csr = 30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;

    // need to poll for the cooldown on the last XIP transfer to expire
    // (via direct-mode BUSY flag) before it is safe to perform the first
    // direct-mode operation
    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) { }

    // exit out of QMI in case we've inited already
    qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;

    // transmit the command to exit QPI quad mode - read ID as standard SPI
    qmi_hw->direct_tx = QMI_DIRECT_TX_OE_BITS |
                        QMI_DIRECT_TX_IWIDTH_VALUE_Q << QMI_DIRECT_TX_IWIDTH_LSB |
                        PSRAM_CMD_QUAD_END;

    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) { }

    (void)qmi_hw->direct_rx;

    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);

    qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
    uint8_t kgd = 0;
    uint8_t eid = 0;
    for (size_t i = 0; i < 7; i++) {
        qmi_hw->direct_tx = (i == 0 ? PSRAM_CMD_READ_ID : PSRAM_CMD_NOOP);

        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_TXEMPTY_BITS) == 0) { }
        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) { }

        if (i == 5)
            kgd = qmi_hw->direct_rx;
        else if (i == 6)
            eid = qmi_hw->direct_rx;
        else
            (void)qmi_hw->direct_rx; // just read and discard
    }

    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

    if(kgd == PSRAM_ID) {
        psram_size = 1024 * 1024;
        uint8_t size_id = eid >> 5;
        if (eid == 0x26 || size_id == 2)
            psram_size *= 8;
        else if (size_id == 0)
            psram_size *= 2;
        else if (size_id == 1)
            psram_size *= 4;
    }
    restore_interrupts(intr_stash);
    return psram_size;
}

// expects interrupts to be enabled on entry
static void __no_inline_not_in_flash_func(set_psram_timing)(void) {

    // Get secs / cycle for the system clock - get before disabling interrupts.
    uint32_t sysHz = (uint32_t)clock_get_hz(clk_sys);

    // Calculate the clock divider - goal to get clock used for PSRAM <= what
    // the PSRAM IC can handle - which is defined in SFE_PSRAM_MAX_SCK_HZ
    volatile uint8_t clockDivider = (sysHz + SFE_PSRAM_MAX_SCK_HZ - 1) / SFE_PSRAM_MAX_SCK_HZ;

    uint32_t intr_stash = save_and_disable_interrupts();

    uint32_t fsPerCycle = SFE_SEC_TO_FS / sysHz;

    // the maxSelect value is defined in units of 64 clock cycles
    // So maxFS / (64 * fsPerCycle) = maxSelect = SFE_PSRAM_MAX_SELECT_FS64/fsPerCycle
    volatile uint8_t maxSelect = SFE_PSRAM_MAX_SELECT_FS64 / fsPerCycle;

    // minDeselect time - in system clock cycle
    // Must be higher than 50ns (min deselect time for PSRAM) so add a fsPerCycle - 1 to round up
    // So minFS/fsPerCycle = minDeselect = SFE_PSRAM_MIN_DESELECT_FS/fsPerCycle
    volatile uint8_t minDeselect = (SFE_PSRAM_MIN_DESELECT_FS + fsPerCycle - 1) / fsPerCycle;

#ifdef HZ30
    qmi_hw->m[1].timing = QMI_M1_TIMING_PAGEBREAK_VALUE_1024 << QMI_M1_TIMING_PAGEBREAK_LSB |
                          0                                  << QMI_M1_TIMING_SELECT_HOLD_LSB |
                          3                                  << QMI_M1_TIMING_COOLDOWN_LSB |
                          0                                  << QMI_M1_TIMING_RXDELAY_LSB |
                          maxSelect                          << QMI_M1_TIMING_MAX_SELECT_LSB |
                          minDeselect                        << QMI_M1_TIMING_MIN_DESELECT_LSB |
       /* clockDivider */ 5                                  << QMI_M1_TIMING_CLKDIV_LSB;
#else
    qmi_hw->m[1].timing = QMI_M1_TIMING_PAGEBREAK_VALUE_1024 << QMI_M1_TIMING_PAGEBREAK_LSB |
                          3 /* 0..3 */                       << QMI_M1_TIMING_SELECT_HOLD_LSB |
                          3 /* 1..3 */                       << QMI_M1_TIMING_COOLDOWN_LSB |
                          7 /* 2..5 */                       << QMI_M1_TIMING_RXDELAY_LSB |
       /* maxSelect */    0x23                               << QMI_M1_TIMING_MAX_SELECT_LSB |
       /* minDeselect */  0x00                               << QMI_M1_TIMING_MIN_DESELECT_LSB |
       /* clockDivider */ 3                                  << QMI_M1_TIMING_CLKDIV_LSB;
       // REVISIT: set that "3" in startup_psram_clock_div
#endif

    restore_interrupts(intr_stash);

    log_write("qmi_hw %x\n", qmi_hw->m[1].timing);
    log_write("qmi_hw %b\n", qmi_hw->m[1].timing);
    log_write("maxSelect=%x minDeselect=%x clockDivider=%x\n", maxSelect, minDeselect, clockDivider);

/*
        +- cooldown
        |  +- pagebreak
        |  |      +- cs delay select hold
        |  |      |      +- max select
        |  |      |      |     +- min deselect
        |  |   *  |      |     | *   +- rx delay
        |  |   |  |      |     | |   |        +- clock divider
       ++ ++ +++ ++ ++++++ +++++ + +++ ++++++++
       fe dc ba9 87 654321 0fedc b a98 76543210
       01 10 000 11 100101 10000 0 001 00000011  0x61cb0103
        1  2      3  2---5 1---0     1        3
       01 10 000 11 010010 01000 0 001 00000010  0x61a48102
        1  2      3  2---2 0---8     1        2
       01 10 000 00 100011 00000 0 010 00000001  0x60460201
        1  2      0  2---3     0     2        1

*/
}

#ifdef PICO_DEFAULT_PSRAM_CS_PIN
static const uint8_t psram_cs_pin = PICO_DEFAULT_PSRAM_CS_PIN;
#else
extern const uint8_t psram_cs_pin;
#endif

size_t __no_inline_not_in_flash_func(psram_init)(){

    gpio_set_function(psram_cs_pin, GPIO_FUNC_XIP_CS1);

    size_t psram_size = get_psram_size();

    if(psram_size == 0) return 0;

    uint32_t intr_stash = save_and_disable_interrupts();

    // Enable quad mode.
    qmi_hw->direct_csr = 30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;

    // Need to poll for the cooldown on the last XIP transfer to expire
    // (via direct-mode BUSY flag) before it is safe to perform the first
    // direct-mode operation
    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) { }

    for (uint8_t i = 0; i < 3; i++) {
        qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
        if (i == 0)
            qmi_hw->direct_tx = PSRAM_CMD_RSTEN;
        else if (i == 1)
            qmi_hw->direct_tx = PSRAM_CMD_RST;
        else
            qmi_hw->direct_tx = PSRAM_CMD_QUAD_ENABLE;

        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) { }
        qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);
        for (size_t j = 0; j < 20; j++) asm("nop");

        (void)qmi_hw->direct_rx;
    }

    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

    restore_interrupts(intr_stash);

    set_psram_timing();

    intr_stash = save_and_disable_interrupts();

    qmi_hw->m[1].rfmt = (QMI_M1_RFMT_PREFIX_WIDTH_VALUE_Q << QMI_M1_RFMT_PREFIX_WIDTH_LSB |
                         QMI_M1_RFMT_ADDR_WIDTH_VALUE_Q << QMI_M1_RFMT_ADDR_WIDTH_LSB |
                         QMI_M1_RFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M1_RFMT_SUFFIX_WIDTH_LSB |
                         QMI_M1_RFMT_DUMMY_WIDTH_VALUE_Q << QMI_M1_RFMT_DUMMY_WIDTH_LSB |
                         QMI_M1_RFMT_DUMMY_LEN_VALUE_24 << QMI_M1_RFMT_DUMMY_LEN_LSB |
                         QMI_M1_RFMT_DATA_WIDTH_VALUE_Q << QMI_M1_RFMT_DATA_WIDTH_LSB |
                         QMI_M1_RFMT_PREFIX_LEN_VALUE_8 << QMI_M1_RFMT_PREFIX_LEN_LSB |
                         QMI_M1_RFMT_SUFFIX_LEN_VALUE_NONE << QMI_M1_RFMT_SUFFIX_LEN_LSB);

    qmi_hw->m[1].rcmd = PSRAM_CMD_QUAD_READ << QMI_M1_RCMD_PREFIX_LSB | 0 << QMI_M1_RCMD_SUFFIX_LSB;

    qmi_hw->m[1].wfmt = (QMI_M1_WFMT_PREFIX_WIDTH_VALUE_Q << QMI_M1_WFMT_PREFIX_WIDTH_LSB |
                         QMI_M1_WFMT_ADDR_WIDTH_VALUE_Q << QMI_M1_WFMT_ADDR_WIDTH_LSB |
                         QMI_M1_WFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M1_WFMT_SUFFIX_WIDTH_LSB |
                         QMI_M1_WFMT_DUMMY_WIDTH_VALUE_Q << QMI_M1_WFMT_DUMMY_WIDTH_LSB |
                         QMI_M1_WFMT_DUMMY_LEN_VALUE_NONE << QMI_M1_WFMT_DUMMY_LEN_LSB |
                         QMI_M1_WFMT_DATA_WIDTH_VALUE_Q << QMI_M1_WFMT_DATA_WIDTH_LSB |
                         QMI_M1_WFMT_PREFIX_LEN_VALUE_8 << QMI_M1_WFMT_PREFIX_LEN_LSB |
                         QMI_M1_WFMT_SUFFIX_LEN_VALUE_NONE << QMI_M1_WFMT_SUFFIX_LEN_LSB);

    qmi_hw->m[1].wcmd = PSRAM_CMD_QUAD_WRITE << QMI_M1_WCMD_PREFIX_LSB | 0 << QMI_M1_WCMD_SUFFIX_LSB;

    xip_ctrl_hw->ctrl |= XIP_CTRL_WRITABLE_M1_BITS;

    restore_interrupts(intr_stash);

    return psram_size;
}

/*
The MIT License (MIT)

Copyright (c) 2024 SparkFun Electronics

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions: The
above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software. The software is provided
"as is", without warranty of any kind, express or implied, including but
not limited to the warranties of merchantability, fitness for a particular
purpose and noninfringement. In no event shall the authors or copyright
holders be liable for any claim, damages or other liability, whether in an
action of contract, tort or otherwise, arising from, out of or in
connection with the software or the use or other dealings in the software.
*/
