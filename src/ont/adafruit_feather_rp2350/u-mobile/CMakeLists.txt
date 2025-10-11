
add_executable(onx-u-mobile
    ${PROJECT_SOURCE_DIR}/src/onl/drivers/radio-rfm69.c
    onx.c
)

target_include_directories(onx-u-mobile PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onl/pico
    ${PROJECT_SOURCE_DIR}/src/ont/images
)

target_link_libraries(onx-u-mobile PRIVATE
    lib_onx
)

pico_enable_stdio_usb(onx-u-mobile 1)
pico_enable_stdio_uart(onx-u-mobile 0)

pico_add_extra_outputs(onx-u-mobile)
