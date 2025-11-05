
add_library(lib_onx STATIC

    ${PICO_BOARD_SOURCES}

    onl/pico/startup.c
    onl/pico/boot.c
    onl/pico/time.c
    onl/pico/dma-mem.c
    onl/pico/log.c
    onl/pico/random.c

    onl/pico/gpio.c
    onl/pico/i2c.c
    onl/pico/spi.c
    onl/pico/usb-host.c

    onl/mem.c
    onl/persistence.c

    onl/tinyusb-host.c
    ${PROJECT_SOURCE_DIR}/sdk-diffs/src/rp2_common/pico_stdio_usb/stdio_usb.c
    ${PROJECT_SOURCE_DIR}/sdk-diffs/src/rp2_common/pico_stdio_usb/stdio_usb_descriptors.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
    ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c

    lib/lib.c
    lib/chunkbuf.c
    lib/colours.c

    lib/database.c
    lib/show_bytes_n_chars.c

    lib/value.c
    lib/list.c
    lib/properties.c

    lib/tests.c

    onn/onn.c
    onp/onp.c
    onp/logic-evaluators.c
    onr/edit-rules.c
)

# foreach(PICO_PIO_TO_COMPILE)...
pico_generate_pio_header(lib_onx
    ${PROJECT_SOURCE_DIR}/${PICO_PIO_TO_COMPILE}
    OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated
)

target_include_directories(lib_onx PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    onl
    onl/pico
    onn
    onp
    ${PICO_SDK_PATH}/src/rp2_common/pico_stdio_usb/include
)

target_compile_definitions(lib_onx PRIVATE
  PIO_USB_USE_TINYUSB
  PICO_STDIO_USB_USE_DEFAULT_DESCRIPTORS=1
)

target_link_libraries(lib_onx PUBLIC
    pico_stdlib
    pico_multicore
    pico_flash
    pico_sync
    pico_rand
    pico_pio_usb
    pico_unique_id
    hardware_sync
    hardware_dma
    hardware_i2c
    hardware_spi
    hardware_gpio
    hardware_pio
    hardware_exception
    hardware_sync
    hardware_adc
    cmsis_core
    tinyusb_device
    tinyusb_host
)


