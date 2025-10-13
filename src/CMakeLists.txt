
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

    onl/mem.c
    onl/persistence.c

    lib/lib.c
    lib/chunkbuf.c
    lib/colours.c
    lib/config.c

    lib/database.c
    lib/show_bytes_n_chars.c

    lib/value.c
    lib/list.c
    lib/properties.c

    lib/tests.c

    onn/onn.c
    onp/onp.c
    onp/behaviours.c
    onr/edit-rules.c
)

# foreach(PICO_PIO_TO_COMPILE)...
pico_generate_pio_header(lib_onx
    ${PROJECT_SOURCE_DIR}/${PICO_PIO_TO_COMPILE}
    OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated
)

target_include_directories(lib_onx PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    onl/pico
    onn
    onp
)

target_link_libraries(lib_onx PUBLIC
    pico_stdlib
    pico_multicore
    pico_flash
    pico_sync
    pico_rand
    hardware_dma
    hardware_i2c
    hardware_spi
    hardware_gpio
    hardware_pio
    hardware_exception
    hardware_sync
    hardware_adc
    cmsis_core
)

pico_enable_stdio_usb(lib_onx 1)
pico_enable_stdio_uart(lib_onx 0)

