
add_executable(onx-o-moon
    ${PROJECT_SOURCE_DIR}/src/onl/seesaw.c
    ${PROJECT_SOURCE_DIR}/src/onl/led-matrix-dotstar.c
    ${PROJECT_SOURCE_DIR}/src/ont/logic-evaluators.c
    ${PROJECT_SOURCE_DIR}/src/ont/io-evaluators.c
    onx.c
)

target_include_directories(onx-o-moon PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/onl
    ${PROJECT_SOURCE_DIR}/src/ont
)

target_link_libraries(onx-o-moon PRIVATE
    lib_onx
)

pico_add_extra_outputs(onx-o-moon)
