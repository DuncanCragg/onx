
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <pico-support.h>
#include <m-class-support.h>

#include <pico/stdio_usb.h>
#include <tusb.h>

#include <onx/lib.h>
#include <onx/boot.h>
#include <onx/mem.h>
#include <onx/time.h>
#include <onx/log.h>
#include <onx/gpio.h>

#include <persistence.h>

#include <onn.h>

#define LOG_BUF_SIZE 1024
static volatile char log_buffer[LOG_BUF_SIZE];
static volatile list* saved_messages = 0;
       volatile list* gfx_log_buffer = 0;

static volatile bool initialised=false;

static log_usb_cb the_log_usb_cb=0;

void log_set_usb_cb(log_usb_cb cb){
  the_log_usb_cb = cb;
}

static volatile char char_recvd=0;

void tud_cdc_rx_cb(uint8_t itf) {
  tud_cdc_read(&char_recvd, 1);
  if(the_log_usb_cb){
    the_log_usb_cb(char_recvd);
    char_recvd=0;
  }
}

void log_init() {

  if(initialised) return;

  saved_messages = list_new(64);
  gfx_log_buffer = list_new(32);

#if defined(RTT_LOG_ENABLED)
  if(log_to_rtt){
    RTT_LOG_INIT(NULL);
    RTT_LOG_DEFAULT_BACKENDS_INIT();
  }
#endif

  if(log_to_std){
    tud_init(0);
    stdio_usb_init();
    for(int i=0; i<500 && !stdio_usb_connected(); i++){
      time_delay_ms(1);
      tud_task();
    }
  }

  if(log_to_led){
    gpio_init();
#define LEDS_ACTIVE_STATE 1 // REVISIT: set in board defs
#if defined(PICO_DEFAULT_LED2_R_PIN)
    gpio_mode(PICO_DEFAULT_LED2_R_PIN, GPIO_MODE_OUTPUT);
    gpio_mode(PICO_DEFAULT_LED2_G_PIN, GPIO_MODE_OUTPUT);
    gpio_mode(PICO_DEFAULT_LED2_B_PIN, GPIO_MODE_OUTPUT);
    gpio_set(PICO_DEFAULT_LED2_R_PIN, !LEDS_ACTIVE_STATE);
    gpio_set(PICO_DEFAULT_LED2_G_PIN, !LEDS_ACTIVE_STATE);
    gpio_set(PICO_DEFAULT_LED2_B_PIN, !LEDS_ACTIVE_STATE);
#elif defined(PICO_DEFAULT_LED_PIN)
    gpio_mode(PICO_DEFAULT_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set(PICO_DEFAULT_LED_PIN,  !LEDS_ACTIVE_STATE);
#endif
    time_init();
    log_flash(0,1,0);
  }

  initialised=true;
}

#define LOG_EARLY_MS 800

#define FLUSH_TO_STDIO  1
#define FLUSH_TO_RTT    2
#define FLUSH_TO_GFX    3

static bool already_in_log_write = false;

// no logging in here obvs
static char* get_reason_to_save_logs(){
  if(already_in_log_write)                 return "LOG ";
  if(time_ms() < LOG_EARLY_MS)             return "ERL ";
  if(in_interrupt_context())               return "INT ";
  if(log_to_std && !stdio_usb_connected()) return "CON ";
  return 0;
}

static void flush_saved_messages(uint8_t to){

  if(get_reason_to_save_logs()) return;

  DISABLE_INTERRUPTS;
  uint16_t ls=list_size(saved_messages);
  list* saved_messages_copy;
  if(ls){
    saved_messages_copy = list_copy(saved_messages);
    list_clear(saved_messages, false);
  }
  ENABLE_INTERRUPTS;
  if(!ls) return;
  for(uint8_t i=1; i<=ls; i++){

    char* msg = list_get_n(saved_messages_copy, i);

    if(to==FLUSH_TO_STDIO){
      if(i==1)  printf("+---- saved messages --------------\n");
      static bool within_line=false;
      if(!within_line) printf("| ");
      printf("%s", msg);
      if(i==ls) printf("+----------------------------------\n");
      within_line=!strchr(msg,'\n');
    }
    if(to==FLUSH_TO_GFX){
      list_add(gfx_log_buffer, msg);
    }
#if defined(RTT_LOG_ENABLED)
    if(to==FLUSH_TO_RTT){
      RTT_LOG_DEBUG("%s", msg);
    }
#endif
    free(msg);
  }
  list_free(saved_messages_copy, false);

#if defined(RTT_LOG_ENABLED)
  if(to==FLUSH_TO_RTT){
    RTT_LOG_FLUSH();
  }
#endif
}

bool log_loop() {

  if(!initialised) return true;

  if(char_recvd){
    log_write(">%c<----------\n", char_recvd);
    if(char_recvd=='c') onn_show_cache();
    if(char_recvd=='n') onn_show_notify();
    if(char_recvd=='v') value_dump_small();
    if(char_recvd=='V') value_dump();
    if(char_recvd=='f') persistence_dump();
    if(char_recvd=='F') persistence_wipe();
    if(char_recvd=='m') mem_show_allocated(true);
    if(char_recvd=='p') gpio_show_power_status();
    if(char_recvd=='r') boot_reset(false);
    if(char_recvd=='b') boot_reset(true);
    if(char_recvd=='*') log_flash(1,1,1);
    if(char_recvd=='h') log_write("c.ache n.otifies Vv.alues f.lash F.ormat m.em p.ower r.eset b.ootloader\n");
    char_recvd=0;
  }

  if(log_to_std){
    flush_saved_messages(FLUSH_TO_STDIO);
  }
  if(log_to_gfx){
    flush_saved_messages(FLUSH_TO_GFX);
  }
#if defined(RTT_LOG_ENABLED)
  if(log_to_rtt){
    flush_saved_messages(FLUSH_TO_RTT);
  }
  return RTT_LOG_PROCESS();
#else
  return false;
#endif
}

#define LOGCHK if(r >= LOG_BUF_SIZE){ log_flash(1,0,0); return 0; }

static int16_t log_write_mode_main(uint8_t mode, char* file, uint32_t line, const char* fmt, va_list args);

int16_t log_write_mode(uint8_t mode, char* file, uint32_t line, const char* fmt, ...){

  // log_write_mode(): in this function WE ONLY USE STD LIB FUNCTIONS
  // so g'tee not re-entering via accidental log_write
  // plus all of this could be in interrupt context, so needs to be light

  va_list args;
  va_start(args, fmt);

  bool fl=(mode==1 || mode==3);
  bool nw=(mode==2 || mode==3);

  // if(!nw) return 0; // narrow down logging to only modes 2/3 REVISIT: have log levels

  int16_t r=0;

  char* save_reason=get_reason_to_save_logs();
  if(save_reason){
    static bool within_line=false;
    if(!within_line){
      r+=    snprintf(log_buffer+r, LOG_BUF_SIZE-r, save_reason);                                          LOGCHK
      r+=fl? snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0; LOGCHK
    }
    r+=     vsnprintf(log_buffer+r, LOG_BUF_SIZE-r, fmt, args);                                            LOGCHK
    within_line=!strchr(log_buffer,'\n');
    char* lb=strdup(log_buffer);
    DISABLE_INTERRUPTS;
    if(!list_add(saved_messages, lb)) free(lb);
    ENABLE_INTERRUPTS;
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
  bool nw=(mode==2 || mode==3);

  int16_t r=0;

  if(log_to_std){
    flush_saved_messages(FLUSH_TO_STDIO);
    r+=fl? printf("[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0;
    r+=    vprintf(fmt, args);
  }
  if(log_to_gfx){
    flush_saved_messages(FLUSH_TO_GFX);
    r+=fl? snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0; LOGCHK
    r+=   vsnprintf(log_buffer+r, LOG_BUF_SIZE-r, fmt, args);                                            LOGCHK
    if(string_is_blank(log_buffer)){
      r=0;
      r+=snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) [blank]", (uint32_t)time_ms(), file, line); LOGCHK
    }
    char* lb=strdup(log_buffer);
    if(!list_add(gfx_log_buffer, lb)) free(lb);
  }
#if defined(RTT_LOG_ENABLED)
  if(log_to_rtt){
    flush_saved_messages(FLUSH_TO_RTT);
    r+=fl? snprintf(log_buffer+r, LOG_BUF_SIZE-r, "[%ld](%s:%ld) ", (uint32_t)time_ms(), file, line): 0; LOGCHK
    r+=   vsnprintf(log_buffer+r, LOG_BUF_SIZE-r, fmt, args);                                            LOGCHK
    RTT_LOG_DEBUG("%s", log_buffer);
    time_delay_ms(2);
    RTT_LOG_FLUSH();
    time_delay_ms(2); // REVISIT
  }
#endif

  return r;
}

static volatile bool    flash_on=false;
static volatile uint8_t flash_nm=0;
static volatile uint8_t flash_r=0;
static volatile uint8_t flash_g=0;
static volatile uint8_t flash_b=0;

static void set_flash_state(){
#if defined(PICO_DEFAULT_LED2_R_PIN)
  if(flash_r) gpio_set(PICO_DEFAULT_LED2_R_PIN, flash_on? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
  if(flash_g) gpio_set(PICO_DEFAULT_LED2_G_PIN, flash_on? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
  if(flash_b) gpio_set(PICO_DEFAULT_LED2_B_PIN, flash_on? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#elif defined(PICO_DEFAULT_LED_PIN)
  ;           gpio_set(PICO_DEFAULT_LED_PIN,  flash_on? LEDS_ACTIVE_STATE: !LEDS_ACTIVE_STATE);
#endif
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
#if defined(RTT_LOG_ENABLED)
  if(log_to_rtt){
    RTT_LOG_FLUSH();
  }
  time_delay_ms(5); // REVISIT
#endif
}


