
add_executable(onx-i-remote
    io-evaluators.c
    onx.c
)

target_include_directories(onx-i-remote PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onn
)

target_link_libraries(onx-i-remote PRIVATE
    pico_sdk_libs
    libonx
)

pico_enable_stdio_usb(onx-i-remote 1)
pico_enable_stdio_uart(onx-i-remote 0)

pico_add_extra_outputs(onx-i-remote)
