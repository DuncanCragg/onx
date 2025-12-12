
cmake_minimum_required(VERSION 3.5)

set(COMPONENTS
  vfs
  esp_driver_usb_serial_jtag
  espressif__cmake_utilities
  espressif__i2c_bus
  waveshare__esp_lcd_jd9365_10_1
  espressif__esp_lcd_touch_gt911
  main
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(onx-u-console)

