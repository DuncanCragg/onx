
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sync-and-mem.h>

#ifdef NRF5
#include <nrf_log.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>
#endif

#include <onx/lib.h>
#include <onx/boot.h>
#include <onx/mem.h>
#include <onx/time.h>
#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/user-io.h>
#include <onx/serial.h>

#include <persistence.h>

#include <onn.h>

extern bool log_arch_init();
extern bool log_arch_loop();
extern bool log_arch_connected();

#define LOG_BUF_SIZE 2048
static volatile char log_buffer[LOG_BUF_SIZE];
static volatile list* saved_messages = 0;
       volatile list* g2d_log_buffer = 0;

static volatile bool initialised=false;

static CRITICAL_SECTION cs;

static log_usb_cb the_log_usb_cb=0;

void log_set_usb_cb(log_usb_cb cb){
  the_log_usb_cb = cb;
}

static volatile char char_recvd=0;

void log_char_recvd(uint8_t ch) {
  char_recvd=ch;
  if(the_log_usb_cb){
    the_log_usb_cb(char_recvd);
    char_recvd=0;
  }
}

void log_init() {

  if(initialised) return;

  CRITICAL_SECTION_INIT(cs);

  if(!saved_messages) saved_messages = list_new(64);

  if(!log_arch_init()) return;

  if(log_to_rtt){
#ifdef NRF5
    NRF_LOG_INIT(0);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("--------- NRF Logging started ---------------");
    NRF_LOG_FLUSH();
    NRF_LOG_PROCESS();
#endif
  }

  if(log_to_g2d){
    g2d_log_buffer = list_new(32);
  }

  if(log_to_led){
    user_io_init(0);
    log_flash(0,1,0);
  }

  initialised=true;
}

#define LOG_EARLY_MS 800

#define FLUSH_TO_STD  1
#define FLUSH_TO_RTT  2
#define FLUSH_TO_G2D  3

static bool already_in_log_write = false;

// no logging in here obvs
static char* get_reason_to_save_logs(){
  if(log_to_std && !log_arch_connected()) return "CON ";
  if(time_ms() < LOG_EARLY_MS)            return "ERL ";
  if(in_interrupt_context())              return "INT ";
  if(already_in_log_write)                return "LOG ";
  return 0;
}

static void flush_saved_messages(uint8_t to){

  if(get_reason_to_save_logs()) return;

  CRITICAL_SECTION_ENTER(cs);
  uint16_t ls=list_size(saved_messages);
  list* saved_messages_copy;
  if(ls){
    saved_messages_copy = list_copy(saved_messages);
    list_clear(saved_messages, false);
  }
  CRITICAL_SECTION_EXIT(cs);
  if(!ls) return;
  for(uint8_t i=1; i<=ls; i++){

    bool free_msg=true;
    char* msg = list_get_n(saved_messages_copy, i);

    if(to==FLUSH_TO_STD){
      if(i==1)  printf("+---- saved messages --------------\n");
      static bool within_line=false;
      if(!within_line) printf("| ");
      printf("%s", msg);
      if(i==ls) printf("+----------------------------------\n");
      within_line=!strchr(msg,'\n');
    }
    if(to==FLUSH_TO_RTT){
#ifdef NRF5
      if(i==1)  NRF_LOG_INFO("+---- saved messages --------------");
      msg[strcspn(msg, "\r\n")]=0;
      NRF_LOG_INFO("| %s", msg);
      if(i==ls) NRF_LOG_INFO("+----------------------------------");
      NRF_LOG_FLUSH();
      NRF_LOG_PROCESS();
#endif
    }
    if(to==FLUSH_TO_G2D){
      free_msg = !list_add(g2d_log_buffer, msg);
    }
    if(free_msg) free(msg);
  }
  list_free(saved_messages_copy, false);
}

_Pragma(__STRING(weak log_user_key_cb))

void log_user_key_cb() {
  log_write("log_user_key_cb()\n");
}

bool log_loop() {

  if(!initialised) return true;

  if(!log_arch_loop()) return false;

  if(char_recvd){
    log_write(">%c<----------\n", char_recvd);
    if(char_recvd=='u') log_user_key_cb();
    if(char_recvd=='c') onn_show_cache();
    if(char_recvd=='n') onn_show_notify();
    if(char_recvd=='v') value_dump_small();
    if(char_recvd=='V') value_dump();
    if(char_recvd=='f') persistence_dump();
    if(char_recvd=='F') persistence_wipe();
    if(char_recvd=='m') mem_show_allocated(true);
    if(char_recvd=='e') log_write("epoch time: %llds\n", time_es());
//  if(char_recvd=='p') gpio_show_power_status(); // PORT
    if(char_recvd=='r') boot_reset(false);
    if(char_recvd=='b') boot_reset(true);
    if(char_recvd=='*') log_flash(1,1,1);
    if(char_recvd=='h') log_write("u.ser key, object c.ache, n.otifies, Vv.alues, f.lash, F.ormat, m.em, e.poch, p.ower, r.eset, b.ootloader\n");
    char_recvd=0;
  }

  if(log_to_std) flush_saved_messages(FLUSH_TO_STD);
  if(log_to_rtt) flush_saved_messages(FLUSH_TO_RTT);
  if(log_to_g2d) flush_saved_messages(FLUSH_TO_G2D);

  return false;
}

#define LOGCHK if(r >= LOG_BUF_SIZE){ log_flash(1,0,0); return 0; }

static int16_t log_write_mode_main(uint8_t mode, char* file, uint32_t line, const char* fmt, va_list args);

int16_t log_write_mode(uint8_t mode, char* file, uint32_t line, const char* fmt, ...){

  // log_write_mode(): in this function WE ONLY USE STD LIB FUNCTIONS
  // so g'tee not re-entering via accidental log_write
  // plus all of this could be in interrupt context, so needs to be light

  int16_t r=0;

  va_list args;
  va_start(args, fmt);

  bool fl=(mode==1 || mode==3);
#ifdef NARROW_LOGGING
  bool nw=(mode==2 || mode==3);
  if(!nw) return 0; // narrow down logging to only modes 2/3 REVISIT: have log levels
#endif

  char* save_reason=get_reason_to_save_logs();
  if(save_reason){
    static bool within_line=false;
    if(!within_line){
      r+=    snprintf(log_buffer+r, LOG_BUF_SIZE-r, save_reason);                                          LOGCHK
      r+=fl? snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0; LOGCHK
    }
    r+=     vsnprintf(log_buffer+r, LOG_BUF_SIZE-r, fmt, args);                                            LOGCHK
    within_line=!strchr(log_buffer,'\n');
    char* msg=strdup(log_buffer);
    if(!saved_messages) saved_messages = list_new(64);
    CRITICAL_SECTION_ENTER(cs);
    if(!list_add(saved_messages, msg)) free(msg);
    CRITICAL_SECTION_EXIT(cs);
    return 0;
  }

  already_in_log_write = true;
  r = log_write_mode_main(mode, file, line, fmt, args);
  already_in_log_write = false;

  va_end(args);

  return r;
}

int16_t log_write_mode_main(uint8_t mode, char* file, uint32_t line, const char* fmt, va_list args){

  if(!initialised) return 0;

  bool fl=(mode==1 || mode==3);
#ifdef NARROW_LOGGING
  bool nw=(mode==2 || mode==3);
#endif

  if(log_to_std){
    flush_saved_messages(FLUSH_TO_STD);
    int16_t r=0;
    r+=fl? printf("[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0;
    r+=    vprintf(fmt, args);
  }
  if(log_to_g2d){
    flush_saved_messages(FLUSH_TO_G2D);
    int16_t r=0;
    r+=fl? snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0; LOGCHK
    r+=   vsnprintf(log_buffer+r, LOG_BUF_SIZE-r, fmt, args);                                            LOGCHK
    if(string_is_blank(log_buffer)){
      r=0;
      r+=snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) [blank]", (uint32_t)time_ms(), file, line); LOGCHK
    }
    char* msg=strdup(log_buffer);
    if(!list_add(g2d_log_buffer, msg)) free(msg);
  }
  if(log_to_rtt){
#ifdef NRF5
    flush_saved_messages(FLUSH_TO_RTT);
    int16_t r=0;
    r+=fl? snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0; LOGCHK
    r+=   vsnprintf(log_buffer+r, LOG_BUF_SIZE-r, fmt, args);                                            LOGCHK
    if(string_is_blank(log_buffer)){
      r=0;
      r+=snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) [blank]", (uint32_t)time_ms(), file, line); LOGCHK
    }
    log_buffer[strcspn(log_buffer, "\r\n")]=0;
    NRF_LOG_INFO("%s", log_buffer);
    time_delay_ms(2); // REVISIT
    NRF_LOG_FLUSH();
    NRF_LOG_PROCESS();
    time_delay_ms(2); // REVISIT
#endif
  }

  return 0; // why return the random chars written to a random channel?
}

static volatile bool    flash_on=false;
static volatile uint8_t flash_nm=0;
static volatile uint8_t flash_r=0;
static volatile uint8_t flash_g=0;
static volatile uint8_t flash_b=0;

static void set_flash_state(){
  uint8_t n=user_io_led_number();
  user_io_led_set(n, flash_on && flash_r, flash_on && flash_g, flash_on && flash_b);
}

#define FLASHES_NUM 3
#define FLASHES_TMS 100

static void flash_time_cb(void*) {
  flash_on=!flash_on;
  flash_nm++;
  set_flash_state();
  if(flash_nm == 2 * FLASHES_NUM){
    flash_nm=0;
    return;
  }
  time_once(flash_time_cb, 0, FLASHES_TMS);
}

void log_flash_current_file_line(char* file, uint32_t line, uint8_t r, uint8_t g, uint8_t b){
#ifdef ONLY_FLASH_111
  if(r+g+b != 3) return;
#endif
  if(!initialised) return;
  if(!strstr(file, "log")){
    log_write_mode(1, file, line, "log_flash\n");
  }
  if(!log_to_led || flash_nm) return;
  flash_r=r; flash_g=g; flash_b=b;
  flash_on=true;
  flash_nm=1;
  set_flash_state();
  time_once(flash_time_cb, 0, FLASHES_TMS);
}

void log_flush() {

  if(!initialised) return;

  if(log_to_rtt){
#ifdef NRF5
    NRF_LOG_FLUSH();
    NRF_LOG_PROCESS();
#endif
  }
}

bool log_connected(){ return log_arch_connected(); }


