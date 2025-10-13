
add_executable(onx-o-moon
    ${PROJECT_SOURCE_DIR}/src/onl/seesaw.c
    ${PROJECT_SOURCE_DIR}/src/onl/led-matrix-dotstar.c
    ${PROJECT_SOURCE_DIR}/src/ont/behaviours.c
    ${PROJECT_SOURCE_DIR}/src/ont/io-evaluators.c
    onx.c
)

target_include_directories(onx-o-moon PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/ont
)

target_link_libraries(onx-o-moon PRIVATE
    lib_onx
)

pico_enable_stdio_usb(onx-o-moon 1)
pico_enable_stdio_uart(onx-o-moon 0)

pico_add_extra_outputs(onx-o-moon)
