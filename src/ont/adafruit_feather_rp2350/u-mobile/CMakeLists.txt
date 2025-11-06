
add_executable(onx-u-mobile
    ${PROJECT_SOURCE_DIR}/src/ont/g2d.c
    ${PROJECT_SOURCE_DIR}/src/ont/g2d-lcd.c
    ${PROJECT_SOURCE_DIR}/src/ont/user-2d.c
    ${PROJECT_SOURCE_DIR}/src/ont/keyboard.c
    ${PROJECT_SOURCE_DIR}/src/ont/io-evaluators.c
    ${PROJECT_SOURCE_DIR}/src/onl/radio-rfm69.c
    onx.c
)

target_include_directories(onx-u-mobile PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onl
    ${PROJECT_SOURCE_DIR}/src/ont
    ${PROJECT_SOURCE_DIR}/src/ont/images
)

target_link_libraries(onx-u-mobile PRIVATE
    lib_onx
)

pico_add_extra_outputs(onx-u-mobile)
