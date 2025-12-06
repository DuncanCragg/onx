
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <inttypes.h>

#include <sdkconfig.h>
#include <soc/soc_caps.h>
#include <esp_rom_sys.h>

#include <esp_private/startup_internal.h>
#include <esp_private/periph_ctrl.h>
#include <esp_private/esp_clk_tree_common.h>

#include <esp_bit_defs.h>
#include <esp_check.h>
#include <esp_psram.h>
#include <esp_heap_caps_init.h>
#include <esp_heap_caps.h>
#include <esp_memory_utils.h>
#include <esp_ldo_regulator.h>
#include <esp_clk_tree.h>
#include <esp_pm.h>
#include <esp_log.h>

#include <onx/boot.h>
#include <onx/random.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/startup.h>
#include <onx/psram.h>

#include <onn.h>

void app_main(void){ while(1); }

void esp_crosscore_int_init(void);

esp_err_t esp_psram_extram_reserve_dma_pool(size_t size);

// priority 1000 means it's the last in the list $esp/components/esp_system/system_init_fn.txt
ESP_SYSTEM_INIT_FN(onx_startup_core_0, SECONDARY, BIT(0), 1000){

  esp_crosscore_int_init();

//heap_caps_enable_nonos_stack_heaps(); // REVISIT: how do you reclaim that?

#if CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL
  if (esp_psram_is_initialized()) {
      esp_err_t r = esp_psram_extram_reserve_dma_pool(CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL);
      if (r != ESP_OK) {
          printf("Could not reserve internal/DMA pool (error 0x%x)\n", r);
          abort();
      }
  }
#endif

  boot_init();

  time_init();

  log_init();

  log_write("=============================== core 0 start ===============================\n");

  random_init();

  onn_init();

  startup_core0_init();

  while(1){
    if(!onn_loop()){       // REVISIT: time onn_loop()
      // time_delay_ms(5); // REVISIT
    }
    startup_core0_loop();
  }
  return 0;
}

ESP_SYSTEM_INIT_FN(onx_startup_core_1, SECONDARY, BIT(1), 1000){

  esp_rom_delay_us(500*1000); // REVISIT: settling time? or sync up to something

  esp_crosscore_int_init();

  printf("\n------------------------------- core 1 start -------------------------------\n");

  startup_core1_init();

  while(1){
    startup_core1_loop();
  }
  return 0;
}
