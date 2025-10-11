
add_executable(onx-o-moon
    onx.c
)

target_include_directories(onx-o-moon PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onn
)

target_link_libraries(onx-o-moon PRIVATE
    lib_onx
)

pico_enable_stdio_usb(onx-o-moon 1)
pico_enable_stdio_uart(onx-o-moon 0)

pico_add_extra_outputs(onx-o-moon)
