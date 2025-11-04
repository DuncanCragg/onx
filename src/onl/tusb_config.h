#ifndef ONX_TUSB_CONFIG_H
#define ONX_TUSB_CONFIG_H

// --------------- USB Device --------------------------

#define CFG_TUD_ENABLED     1
#define CFG_TUD_CDC         1

#ifndef CFG_TUD_CDC_RX_BUFSIZE
#define CFG_TUD_CDC_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 256)
#endif

#ifndef CFG_TUD_CDC_TX_BUFSIZE
#define CFG_TUD_CDC_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 256)
#endif

#ifndef CFG_TUD_CDC_EP_BUFSIZE
#define CFG_TUD_CDC_EP_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
#endif

// --------------- USB Host ----------------------------

#define CFG_TUH_ENABLED             1
#define CFG_TUH_RPI_PIO_USB         1
#define CFG_TUH_ENUMERATION_BUFSIZE 256
#define CFG_TUH_HUB                 1
#define CFG_TUH_DEVICE_MAX          (CFG_TUH_HUB ? 4 : 1) // REVISIT: 4-port hub
#define CFG_TUH_HID                  4
#define CFG_TUH_HID_EPIN_BUFSIZE    64
#define CFG_TUH_HID_EPOUT_BUFSIZE   64

#endif
