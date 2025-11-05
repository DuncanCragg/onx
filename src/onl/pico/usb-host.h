#ifndef USB_HOST_H
#define USB_HOST_H

extern const int8_t usb_host_pio_data_plus_pin;
extern const int8_t usb_host_pio_dma_channel;
extern const int8_t usb_host_pio_enable_pin;

void usb_host_init();
void usb_host_loop();

#endif
