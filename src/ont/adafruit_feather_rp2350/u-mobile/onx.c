
#include <stdint.h>
#include <stdbool.h>

// -----------------------------------------------------

const int8_t usb_host_pio_data_plus_pin = 24;
const int8_t usb_host_pio_dma_channel   =  9;
const int8_t usb_host_pio_enable_pin    = -1;

const uint8_t radio_rfm69_cs_pin  = 6;
const uint8_t radio_rfm69_rst_pin = 9;
const uint8_t radio_rfm69_int_pin = 5;

const bool log_to_std = true;
const bool log_to_gfx = false;
const bool log_to_rtt = false;
const bool log_to_led = true;

const bool  onp_log         = true;
const char* onp_channels    = "radio";
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = 0;

// -----------------------------------------------------

