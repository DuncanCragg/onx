
#include <pico/stdlib.h>
#include <pio_usb.h>
#include <tusb.h>

#include <onx/gpio.h>
#include <onx/time.h>
#include <onx/log.h>

#include "usb-host.h"

static bool initialised=false;

void usb_host_init(){

  if(usb_host_pio_enable_pin >= 0){
    gpio_mode(usb_host_pio_enable_pin, GPIO_MODE_OUTPUT);
    gpio_set( usb_host_pio_enable_pin, 1);
  }
  if(usb_host_pio_data_plus_pin >= 0){
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = usb_host_pio_data_plus_pin;
    pio_cfg.tx_ch  = usb_host_pio_dma_channel;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
    tuh_init(1);
    initialised=true;
  }
}

void usb_host_loop(){
  if(!initialised) return;
  tuh_task_ext(0,0);
}






