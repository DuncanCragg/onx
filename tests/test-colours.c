
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <onx/log.h>
#include <onx/colours.h>

#include <tests.h>

void run_colour_tests(bool run_leds){

  log_write("-------- colour tests ---------\n");

  colours_bcs bcs;
  colours_hsv hsv;
  colours_rgb rgb;

  bcs = (colours_bcs){ 255,  85,  64 }; rgb = colours_bcs_to_rgb(bcs);
  log_write("bcs=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", bcs.b, bcs.c, bcs.s, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85, 191, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){   0,   0,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){   0,   0, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){   0,   0, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){  85,   0,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85,   0, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85,   0, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){ 171,   0,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){ 171,   0, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){ 171,   0, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){   0, 127,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){   0, 127, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){   0, 127, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){  85, 127,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85, 127, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85, 127, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){ 171, 127,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){ 171, 127, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){ 171, 127, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){   0, 255,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){   0, 255, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){   0, 255, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){  85, 255,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85, 255, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){  85, 255, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  hsv = (colours_hsv){ 171, 255,   0 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){ 171, 255, 127 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);
  hsv = (colours_hsv){ 171, 255, 255 }; rgb = colours_hsv_to_rgb(hsv);
  log_write("hsv=(%3d,%3d,%3d): rgb=(%3d,%3d,%3d)\n", hsv.h, hsv.s, hsv.v, rgb.r, rgb.g, rgb.b);

  char* colour_chars;
  colours_rgb from_chars;

  colour_chars = "%ffff00"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);
  colour_chars = "%ff5500"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);
  colour_chars = "%ffab00"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);
  colour_chars = "%7f557f"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);

  colour_chars = "#ffffff"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);
  colour_chars = "#55ffff"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);
  colour_chars = "#abffff"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);
  colour_chars = "#557f7f"; from_chars = colours_parse_string(colour_chars);
  log_write("%s = rgb: { %02x %02x %02x }\n", colour_chars, from_chars.r, from_chars.g, from_chars.b);

  if(!run_leds) return;
/* REVISIT when API is in...
  led_strip_fill_col( "#ff0");
  led_matrix_fill_col("#ff0");  led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_col( "#f0f");
  led_matrix_fill_col("#f0f");  led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_col( "#0ff");
  led_matrix_fill_col("#0ff");  led_strip_show(); led_matrix_show(); time_delay_ms(350);

  led_strip_fill_col( "red");
  led_matrix_fill_col("red");   led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_col( "green");
  led_matrix_fill_col("green"); led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_col( "blue");
  led_matrix_fill_col("blue");  led_strip_show(); led_matrix_show(); time_delay_ms(350);

  led_strip_fill_rgb( (colours_rgb){ 255,255,  0 });
  led_matrix_fill_rgb((colours_rgb){ 255,255,  0 }); led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_rgb( (colours_rgb){ 255,  0,255 });
  led_matrix_fill_rgb((colours_rgb){ 255,  0,255 }); led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_rgb( (colours_rgb){   0,255,255 });
  led_matrix_fill_rgb((colours_rgb){   0,255,255 }); led_strip_show(); led_matrix_show(); time_delay_ms(350);

  led_strip_fill_hsv( (colours_hsv){   0,255,127 });
  led_matrix_fill_hsv((colours_hsv){   0,255,127 }); led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_hsv( (colours_hsv){  85,255,127 });
  led_matrix_fill_hsv((colours_hsv){  85,255,127 }); led_strip_show(); led_matrix_show(); time_delay_ms(350);
  led_strip_fill_hsv( (colours_hsv){ 171,255,127 });
  led_matrix_fill_hsv((colours_hsv){ 171,255,127 }); led_strip_show(); led_matrix_show(); time_delay_ms(350);
*/
}


