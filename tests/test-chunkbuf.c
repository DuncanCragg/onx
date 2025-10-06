
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <onx/log.h>
#include <onx/chunkbuf.h>

#include <tests.h>

void run_chunkbuf_tests(){

  log_write("-------- chunkbuf tests ---------\n");

  chunkbuf* wside = chunkbuf_new(130+20*4+22, true);
  chunkbuf* rside = chunkbuf_new(130+20*4+22, true);

  // this exact string has a checksum that's \n
  #define CHKSUM_IS_NL "UID: uid-ab3c-ae72-bf87-7c00 Ver: 2 Devices: uid-c124-447c-ad00-e529 Notify: uid-5cef-4db6-79f0-0197 is: editable light light: off"
  tests_assert(chunkbuf_writable(wside, strlen(CHKSUM_IS_NL), '\n'), "wside writable");
  chunkbuf_write(wside, CHKSUM_IS_NL, strlen(CHKSUM_IS_NL), '\n');

  tests_assert(chunkbuf_writable(wside, strlen("1234567890123456789"), '\n'), "wside writable");
  chunkbuf_write(wside, "1234567890123456789", strlen("1234567890123456789"), '\n');

  tests_assert(chunkbuf_writable(wside, strlen("5678901234567891234"), '\n'), "wside writable");
  chunkbuf_write(wside, "5678901234567891234", strlen("5678901234567891234"), '\n');

  tests_assert(chunkbuf_writable(wside, strlen("1234567890123456789"), '\n'), "wside writable");
  chunkbuf_write(wside, "1234567890123456789", strlen("1234567890123456789"), '\n');

  tests_assert(chunkbuf_writable(wside, strlen("5678901234567891234"), '\n'), "wside writable");
  chunkbuf_write(wside, "5678901234567891234", strlen("5678901234567891234"), '\n');

  tests_assert( chunkbuf_writable(wside, 15,   -1), "wside writable for 15 more no delim");
  tests_assert(!chunkbuf_writable(wside, 15, '\n'), "wside not writable for 15 more w. delim");

  log_write("-------- chunkbuf written -----------\n");

  for(int i=0; ; i++){
    char pkt[7];
    int n=216-i*7;
    if(n>0) tests_assert_equal_num(chunkbuf_readable(wside, -1), n, "wside readable");
    uint16_t rn = chunkbuf_read(wside, pkt, 7, -1);
  ; if(!rn) break;
    if(i==23){ log_write("ohh nooo! packet loossss!! %^#!&*~ (%.7s)\n", pkt); continue; }
    if(i==27){ log_write("ohh nooo! corrupptiooion!! %^#!&*~\n"); pkt[1]='#'; }
    tests_assert(chunkbuf_writable(rside, rn, -1), "---- rside writable");
    chunkbuf_write(rside, pkt, rn, -1);
  }
  tests_assert( chunkbuf_writable(rside, 15+7, -1), "---- rside writable 15+7 more");
  tests_assert(!chunkbuf_writable(rside, 15+8, -1), "---- rside not writable 15+8 more");

  log_write("-------- chunkbuf transferred ---\n");

  for(int l=1; ; l++){
    char line[132];
    uint16_t rd = chunkbuf_readable(rside, '\n');
  ; if(!rd) break;
    tests_assert_equal_num(rd, l==1? 132: l==3? 14: 21, "---- rside readable (wrong when pkt lost)");
    uint16_t rn = chunkbuf_read(rside, line, 132, '\n');
    log_write("rn=%d line: \"%s\" len: %d\n", rn, line, strlen(line));
    if(l==1) tests_assert_equal_num(rn, 130, "first line is 130 long");
    else     tests_assert_equal_num(rn, l==3 || l==4? 0: 19,  "next lines are 19 long unless corrupt");
  }

  log_write("-------- chunkbuf done ----------\n");

  chunkbuf_free(rside);
  chunkbuf_free(wside);
}


