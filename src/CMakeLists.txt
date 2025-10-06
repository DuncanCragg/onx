
add_library(libonx

    ${PICO_BOARD_SOURCES}

    onl/pico/startup.c
    onl/pico/boot.c
    onl/pico/time.c
    onl/pico/mem.c
    onl/pico/dma-mem.c
    onl/pico/log.c
    onl/pico/random.c

    onl/pico/gpio.c
    onl/pico/spi.c

    onl/pico/spi-flash.c
    onl/pico/persistence.c

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
)

target_include_directories(libonx PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    onl/pico
    onn
    onp
)

target_link_libraries(libonx PRIVATE
    pico_sdk_libs
    cmsis_core
)

pico_enable_stdio_usb(libonx 1)
pico_enable_stdio_uart(libonx 0)

