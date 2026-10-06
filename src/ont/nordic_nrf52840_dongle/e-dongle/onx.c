
#include <onx/time.h>
#include <onx/random.h>
#include <onx/log.h>
#include <onx/serial.h>

#include <onn.h>

const bool log_to_std = true;
const bool log_to_g2d = false;
const bool log_to_rtt = false;
const bool log_to_led = false;

const bool  onp_log         = false;
const char* onp_channels    = "radio serial";
const char* onp_serial_ttys = 0;
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = "pcr-";

void startup_core0_init(){
  log_write("\n------Starting PCR Test Server-----\n");
}

void startup_core0_loop(){
}

// --------------------------------------------------------------------


