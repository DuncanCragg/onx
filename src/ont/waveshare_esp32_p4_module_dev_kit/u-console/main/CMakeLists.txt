idf_component_register(
  SRCS
    "onx.c"
    "../../../../onl/esp32/startup.c"
    "../../../../onl/esp32/dsi-jd9365.c"

  INCLUDE_DIRS "../../../../../include"
  REQUIRES
)

idf_build_set_property(C_COMPILE_OPTIONS "-Wno-discarded-qualifiers" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Wno-pointer-sign"         APPEND)

















