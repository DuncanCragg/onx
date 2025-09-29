
// --------------------------------------------------------------------

#include <string.h>

#include <onex-kernel/random.h>
#include <onex-kernel/time.h>
#include <onex-kernel/log.h>

#include <onex-kernel/chunkbuf.h>
#include <onex-kernel/colours.h>
#include <persistence.h>
#include <onn.h>

#include <items.h>

#include <tests.h>

extern void run_value_tests();
extern void run_list_tests();
extern void run_properties_tests();

extern void run_database_tests(properties* config);
extern void run_colour_tests();
void run_chunkbuf_tests();
extern void run_onn_tests(properties* config);

void run_tests(properties* config) {

  log_write("-----------------OnexKernel tests------------------------\n");

  run_value_tests();
  run_list_tests();
  run_properties_tests();

  run_database_tests(config);
  run_chunkbuf_tests();

  run_onn_tests(config);

  onex_assert_summary();

  run_colour_tests();
}

void run_chunkbuf_tests(){

  log_write("-------- chunkbuf tests ---------\n");

  chunkbuf* wside = chunkbuf_new(130+20*4+22, true);
  chunkbuf* rside = chunkbuf_new(130+20*4+22, true);

  // this exact string has a checksum that's \n
  #define CHKSUM_IS_NL "UID: uid-ab3c-ae72-bf87-7c00 Ver: 2 Devices: uid-c124-447c-ad00-e529 Notify: uid-5cef-4db6-79f0-0197 is: editable light light: off"
  onex_assert(chunkbuf_writable(wside, strlen(CHKSUM_IS_NL), '\n'), "wside writable");
  chunkbuf_write(wside, CHKSUM_IS_NL, strlen(CHKSUM_IS_NL), '\n');

  onex_assert(chunkbuf_writable(wside, strlen("1234567890123456789"), '\n'), "wside writable");
  chunkbuf_write(wside, "1234567890123456789", strlen("1234567890123456789"), '\n');

  onex_assert(chunkbuf_writable(wside, strlen("5678901234567891234"), '\n'), "wside writable");
  chunkbuf_write(wside, "5678901234567891234", strlen("5678901234567891234"), '\n');

  onex_assert(chunkbuf_writable(wside, strlen("1234567890123456789"), '\n'), "wside writable");
  chunkbuf_write(wside, "1234567890123456789", strlen("1234567890123456789"), '\n');

  onex_assert(chunkbuf_writable(wside, strlen("5678901234567891234"), '\n'), "wside writable");
  chunkbuf_write(wside, "5678901234567891234", strlen("5678901234567891234"), '\n');

  onex_assert( chunkbuf_writable(wside, 15,   -1), "wside writable for 15 more no delim");
  onex_assert(!chunkbuf_writable(wside, 15, '\n'), "wside not writable for 15 more w. delim");

  log_write("-------- chunkbuf written -----------\n");

  for(int i=0; ; i++){
    char pkt[7];
    int n=216-i*7;
    if(n>0) onex_assert_equal_num(chunkbuf_readable(wside, -1), n, "wside readable");
    uint16_t rn = chunkbuf_read(wside, pkt, 7, -1);
  ; if(!rn) break;
    if(i==23){ log_write("ohh nooo! packet loossss!! %^#!&*~ (%.7s)\n", pkt); continue; }
    if(i==27){ log_write("ohh nooo! corrupptiooion!! %^#!&*~\n"); pkt[1]='#'; }
    onex_assert(chunkbuf_writable(rside, rn, -1), "---- rside writable");
    chunkbuf_write(rside, pkt, rn, -1);
  }
  onex_assert( chunkbuf_writable(rside, 15+7, -1), "---- rside writable 15+7 more");
  onex_assert(!chunkbuf_writable(rside, 15+8, -1), "---- rside not writable 15+8 more");

  log_write("-------- chunkbuf transferred ---\n");

  for(int l=1; ; l++){
    char line[132];
    uint16_t rd = chunkbuf_readable(rside, '\n');
  ; if(!rd) break;
    onex_assert_equal_num(rd, l==1? 132: l==3? 14: 21, "---- rside readable (wrong when pkt lost)");
    uint16_t rn = chunkbuf_read(rside, line, 132, '\n');
    log_write("rn=%d line: \"%s\" len: %d\n", rn, line, strlen(line));
    if(l==1) onex_assert_equal_num(rn, 130, "first line is 130 long");
    else     onex_assert_equal_num(rn, l==3 || l==4? 0: 19,  "next lines are 19 long unless corrupt");
  }

  log_write("-------- chunkbuf done ----------\n");

  chunkbuf_free(rside);
  chunkbuf_free(wside);
}

void run_colour_tests(){

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
}

int main() {

  properties* config = properties_new(32);
  properties_set(config, "db-path", value_new("tests.ondb"));
  properties_set(config, "channels", list_vals_new_from_fixed("serial ipv6"));
  properties_set(config, "flags", list_vals_new_from_fixed("db-format"));
  properties_set(config, "test-uid-prefix", value_new("tests"));

  time_init();

  log_init(config);
  log_write("-------- test of early message 1 -------\n");

  random_init();

  run_tests(config);

  time_end();

  properties_free(config, true);
}

// --------------------------------------------------------------------
