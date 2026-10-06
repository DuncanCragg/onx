
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <onx/boot.h>
#include <onx/random.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/serial.h>
#include <onx/startup.h>
#include <onx/psram.h>

#include <onn.h>

int main() {

  // boot_init(); // watchdog!

  time_init();

  log_init();

  log_write("=============================== core 0 start ===============================\n");

  random_init();

  onn_init();

  startup_core0_init();

  while(1){
    if(!onn_loop()){     // REVISIT: time onn_loop()
    // time_delay_ms(5); // REVISIT
    }
    startup_core0_loop();
  }
}


