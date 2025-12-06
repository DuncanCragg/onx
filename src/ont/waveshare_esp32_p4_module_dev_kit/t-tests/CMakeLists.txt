
cmake_minimum_required(VERSION 3.5)

set(COMPONENTS
  vfs
  esp_driver_usb_serial_jtag
  esp_psram
  main
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(onx-t-tests)

