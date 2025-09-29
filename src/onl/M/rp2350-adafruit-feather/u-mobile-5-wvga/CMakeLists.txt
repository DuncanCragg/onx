add_executable(onx
        onx.c
        sfe_psram.c
        dma-mem.c
        ${CMAKE_CURRENT_LIST_DIR}/../../../../ont/M/hx.c
        )

target_include_directories(onx PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../include
        ${CMAKE_CURRENT_LIST_DIR}/../../../../ont/images
        )

target_link_libraries(onx
        pico_stdlib pico_multicore pico_flash pico_sync
        hardware_dma hardware_spi hardware_gpio hardware_exception hardware_sync
        )

pico_enable_stdio_usb(onx 1)
pico_enable_stdio_uart(onx 0)

pico_add_extra_outputs(onx)
