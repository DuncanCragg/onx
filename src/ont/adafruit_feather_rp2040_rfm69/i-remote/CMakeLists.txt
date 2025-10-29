
add_executable(onx-i-remote
    ${PROJECT_SOURCE_DIR}/src/onl/seesaw.c
    ${PROJECT_SOURCE_DIR}/src/ont/logic-evaluators.c
    ${PROJECT_SOURCE_DIR}/src/ont/io-evaluators.c
    onx.c
)

target_include_directories(onx-i-remote PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onn
    ${PROJECT_SOURCE_DIR}/src/ont
)

target_link_libraries(onx-i-remote PRIVATE
    lib_onx
)

pico_enable_stdio_usb(onx-i-remote 1)
pico_enable_stdio_uart(onx-i-remote 0)

pico_add_extra_outputs(onx-i-remote)
