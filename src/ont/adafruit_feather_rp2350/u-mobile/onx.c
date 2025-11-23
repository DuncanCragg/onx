
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sync-and-mem.h>

#include <onx/log.h>
#include <onx/dma-mem.h>
#include <onx/hstx.h>
#include <onx/items.h>
#include <onx/io.h>

#include <onn.h>
#include <onr.h>
#include <ont.h>

#include <user-2d.h>

#include <g2d.h>

// -----------------------------------------------------

const int8_t usb_host_pio_data_plus_pin = 24;
const int8_t usb_host_pio_dma_channel   =  9;
const int8_t usb_host_pio_enable_pin    = -1;

const uint8_t radio_rfm69_cs_pin  = 6;
const uint8_t radio_rfm69_rst_pin = 9;
const uint8_t radio_rfm69_int_pin = 5;

const bool log_to_std = true;
const bool log_to_gfx = false;
const bool log_to_rtt = false;
const bool log_to_led = true;

const bool  onp_log         = true;
const char* onp_channels    = "radio";
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = 0;

// -----------------------------------------------------

char* useruid;
char* homeuid;
char* inventoryuid;

object* user;
object* responses;

volatile bool     button_pressed=false;

volatile uint8_t  pending_user_event;
volatile uint32_t pending_user_event_time;

// -----------------------------------------------------

#define RGB_BYTES_TO_RGB555(r,g,b) (((r)&0b11111000)<<7)|(((g)&0b11111000)<<2)|(((b)&0b11111000)>>3)

// RGB565 to RGB555: ((pixel >> 1) & 0x7fe0) | (pixel & 0x001f);

// -----------------------------------------------------

#define DO_PHOTO_FRAME // DO_PHOTO_FRAME

#ifdef  DO_PHOTO_FRAME

#define NO_ALL_SPRITES // DO_ALL_SPRITES
#define DO_IMAGE_PANEL // DO_IMAGE_PANEL
#define NO_WALLPAPER   // DO_WALLPAPER
#define NO_G2D         // DO_G2D
#define SCROLL_SPEED 0
#define NUM_SPRITES  1

#else

#define DO_ALL_SPRITES // DO_ALL_SPRITES
#define DO_IMAGE_PANEL // DO_IMAGE_PANEL
#define DO_WALLPAPER   // DO_WALLPAPER
#define DO_G2D         // DO_G2D
#define SCROLL_SPEED 1
#define NUM_SPRITES  4

#endif

#define DO_TIME_PSRAM  // DO_TIME_PSRAM
#define DO_FRAME_TIME  // DO_FRAME_TIME
#define DO_INTERLACING 1

volatile bool scenegraph_write=false;

#define scenegraph_read (!scenegraph_write)

typedef struct sprite {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint16_t c;
} sprite;

#ifdef DO_PHOTO_FRAME
#define SPRITE_DEF_IMG \
  { 140,   0, 940, 800, 0b1000000000000000 },
#else
#define SPRITE_DEF_IMG \
  {   0,   0, 200, 800, 0b1000000000000000 },
#endif

#define SPRITE_DEFS                         \
  SPRITE_DEF_IMG                            \
  { 350, 300,  90,  90, 0b0111100111111111 }, \
  { 100, 100,  90,  90, 0b0001110011100111 }, \
  { 150, 100,  30,  30, 0b0111111111100111 }, \
  { 250, 200,  60,  60, 0b0001111111111111 }, \
  { 350, 350,  90,  90, 0b0000000000000000 }, \
  { 200, 200, 300, 200, 0b0111111111111111 }, \
  { 250, 200, 300, 200, 0b0111111111111111 }, \
  { 300, 300,  90,  90, 0b0111100111111111 }, \
  { 100, 100,  30,  30, 0b0111111111100111 }, \
  { 200, 200,  60,  60, 0b0001111111111111 }, \
  { 300, 300,  90,  90, 0b0111111111100000 }  \

static sprite scenegraph[2][12] = { { SPRITE_DEFS },{ SPRITE_DEFS } };

// ------------------------------------------------------------------------

#define CHUNXIZE 320
#define NUMCHUNX (798*(1280/CHUNXIZE)) // 3192

uint16_t   sram_buffer[CHUNXIZE];
uint16_t* psram_buffer =  (uint16_t*)0x15000000;

void __not_in_flash_func(write_image_chunk_to_psram)(uint32_t offset, uint32_t length){
  memcpy(psram_buffer+offset, sram_buffer, length*2);
}

static volatile bool alternate_image=false;

// simulates filling the sram_buffer with image data, slowly, from SD card
// alternate_image determines how it looks, so is the image "ID" from two choices
void __not_in_flash_func(write_image_chunk_to_sram)(uint32_t offset, uint32_t length){
  uint8_t d=alternate_image? 4: 3;
  uint32_t n=length;
  int ystart=offset / H_RESOLUTION;
  int xstart=offset % H_RESOLUTION;
  for(int y=ystart; n; y++){
    int line_offset = (y * H_RESOLUTION);
    for(int x=xstart; n; x++){
      uint32_t i=x+line_offset;
      uint16_t pixel;
      if(y<V_RESOLUTION*1/d) pixel = d==4? (i/2 & 0x03fe): (i & 0x7c00);
      else
      if(y<V_RESOLUTION*2/d) pixel = d==4? (i/2 & 0x7fd0): (i & 0x03e0);
      else
                             pixel = d==4? (i/2 & 0x7ede): (i & 0x001f);
      sram_buffer[length-n] = pixel;
      if(n % 4) time_delay_us(1);
      n--;
    }
  }
}

static volatile bool sram_writable=false;
static volatile bool sram_readable=false;
static volatile uint32_t chunk_number=0;

// trigger a pipeline pushing "SD card images" via sram to psram to show on-screen
// set alternate_image to the "ID" of the image we want
void log_user_key_cb(){
  if(sram_readable) return;
  alternate_image=!alternate_image;
  chunk_number=0;
  sram_writable=true;
}

void __not_in_flash_func(write_next_chunk_to_sram)(){
  static int64_t s=0;
  static int64_t e=0;
  if(chunk_number==0){
    s=time_us();
  }
  if(chunk_number==NUMCHUNX){
    sram_writable=false;
    e=time_us();
    log_write("copy done in %.3llums\n", (e-s)/1000);
;   return;
  }
  write_image_chunk_to_sram(chunk_number*CHUNXIZE, CHUNXIZE);
  sram_readable=true;
  sram_writable=false;
}

void __not_in_flash_func(write_next_chunk_to_psram)(){
  write_image_chunk_to_psram(chunk_number*CHUNXIZE, CHUNXIZE);
  sram_readable=false;
  sram_writable=true;
  chunk_number++;
}

// ------------------------------------------------------------------------

static bool touch_down=false;

static void io_cb() {
  if(io.touched){
    touch_down = true;
    g2d_touch_event(true, io.touch_x, io.touch_y);
  }
  else
  if(touch_down){ // you can get >1 touch up event so reduce to just one
    touch_down=false;
    g2d_touch_event(false, io.touch_x, io.touch_y);
  }
  // simulate physical back button with bottom-left of screen
  if(io.touched && !button_pressed){
    if(io.touch_x < 240 && io.touch_y > 320){
      button_pressed=true;
      onn_run_evaluators(useruid, (void*)USER_EVENT_BUTTON);
    }
  }
  else
  if(!io.touched && button_pressed){
    button_pressed = false;
    onn_run_evaluators(useruid, (void*)USER_EVENT_BUTTON);
  }
}

// ------------------------------------------------------------------------

static char note_text[] = "the fat cat sat on me";

static char note_text_big[] =
  "xxxxxxxxxxxxxxxxxxx " "xxxxxxxxxxxxxxxxxx " "xxxxxxxxxxxxxxxx " "xxxxxxxxxxxxxxx "
  "xxxxxxxxxxxxxx " "xxxxxxxxxxxxx " "xxxxxxxxxxxx " "xxxxxxxxxxx " "xxxxxxxxxx "
  "xxxxxxxxx " "xxxxxxxx " "xxxxxxx " "xxxxxx " "xxxxx " "xxxx " "xxx " "xx " "x "
  "Welcome to ONX! " "and the Object Network " "A Smartwatch OS " "Without Apps "
  "app-killer, inversion " "only see data in HX " "no apps, like the Web " "all our data "
  "just stuff - objects " "you can link to and list " "little objects "
  "of all kinds of data " "linked together " "semantic " "on our own devices "
  "hosted by you (including you) " "sewn together " "into a global, shared fabric "
  "which we can all Link up " "into a shared global data fabric " "like the Web " "mesh "
  "see objects inside other watches " "add their objects to your lists "
  "internet after mesh " "we create a global data fabric "
  "from all our objects linked up " "a two-way dynamic data Web " "a global Meshaverse "
  "chat Freedom Meshaverse " "spanning the planet " "animated by us "
  "internally-animated " "programmed like a spreadsheet "
  "objects are live - you see them change " "you have live presence as an object yourself "
  "SS-like PL over objects as objects themselves " "can share rule object set objects "
  "like 'downloading an app' " "internally-animated "
  "with behaviour rules you can write yourself "
  "and animate ourselves with spreadsheet-like rules "
  "----- ----- ----- ----- -----";

bool evaluate_user(object* usr, void* d) {
  return evaluate_user_2d(usr, d);
}

bool evaluate_default(object* obj, void* d) {
  log_write("evaluate_default d=%p\n", d);
  return true;
}

void init_onx(){

  log_write("Starting ONX.....\n");

  onn_set_evaluators("eval_default",   evaluate_edit_rule, evaluate_default, 0);
  onn_set_evaluators("eval_editable",  evaluate_edit_rule, 0);
  onn_set_evaluators("eval_user",                          evaluate_user, 0);
  onn_set_evaluators("eval_notes",     evaluate_edit_rule, 0);

  object* home;
  object* allobjects;
  object* inventory;
  object* note1;
  object* note2;
  object* notes;

  char* allobjectsuid;
  char* responsesuid;
  char* deviceuid;
  char* note1uid;
  char* note2uid;
  char* notesuid;

  object* uid_0=onn_get_from_cache("uid-0");
  if(!uid_0){

    user      =object_new(0, "eval_user",      "user", 8);
    responses =object_new(0, "eval_default",   "user responses", 12); // REVISIT "editable"?
    home      =object_new(0, "eval_editable",  "list editable", 4);
    allobjects=object_new(0, "eval_editable",  "list editable", 4);
    inventory =object_new(0, "eval_editable",  "list editable", 4);
    note1     =object_new(0, "eval_notes",     "text editable", 4);
    note2     =object_new(0, "eval_notes",     "text editable", 4);
    notes     =object_new(0, "eval_notes",     "text list editable", 4);

    deviceuid   =object_property(onn_device_object, "UID");
    useruid     =object_property(user, "UID");
    responsesuid=object_property(responses, "UID");

    homeuid      =object_property(home, "UID");
    allobjectsuid=object_property(allobjects, "UID");
    inventoryuid =object_property(inventory, "UID");

    note1uid     =object_property(note1, "UID");
    note2uid     =object_property(note2, "UID");
    notesuid     =object_property(notes, "UID");

    object_property_set(user, "responses", responsesuid);
    object_property_set(user, "inventory", inventoryuid);

    char* strtok_state = 0;
    char* word = strtok_r(note_text, " ", &strtok_state);
    while(word){
      object_property_add(note1, "text", word);
      word = strtok_r(0, " ", &strtok_state);
    }
    strtok_state = 0;
    word = strtok_r(note_text_big, " ", &strtok_state);
    while(word){
      object_property_add(note2, "text", word);
      word = strtok_r(0, " ", &strtok_state);
    }
    object_property_set(notes, "title", "Notes");
    object_property_add(notes, "list", note1uid);
    object_property_add(notes, "list", note2uid);

    object_property_set(home, "title", "Home");
    object_property_add(home, "list", allobjectsuid);
    object_property_add(home, "list", notesuid);

    object_property_set(allobjects, "title", "All objects");
    object_property_add(allobjects, "list", deviceuid);
    object_property_add(allobjects, "list", homeuid);
    object_property_add(allobjects, "list", inventoryuid);
    object_property_add(allobjects, "list", notesuid);
    object_property_add(allobjects, "list", useruid);
    object_property_add(allobjects, "list", note1uid);
    object_property_add(allobjects, "list", note2uid);
    object_property_add(allobjects, "list", responsesuid);

    object_property_set(inventory, "title", "Inventory");

    object_property_set(user, "viewing", allobjectsuid);

    object_property_set(onn_device_object, "name", "MobCon5");
    object_property_add(onn_device_object, "user", useruid);

    uid_0=object_new("uid-0", 0, "config", 10);
    object_property_set(uid_0, "user", useruid);

  } else {

    useruid = object_property(uid_0, "user");

    user = onn_get_from_cache(useruid);
  }
  onn_run_evaluators(useruid, (void*)USER_EVENT_INITIAL);
}

// ------------------------------------------------------------------------

void ont_hx_init(){

  io_init(io_cb);

  g2d_init();

  init_onx();
}

uint32_t loop_time=0;

static volatile int yoff=0;

static bool even_lines=true;

void __not_in_flash_func(ont_hx_frame)(bool new_frame){

  if(sram_writable) write_next_chunk_to_sram();

; if(!new_frame) return;

  even_lines = !even_lines;

  yoff+=SCROLL_SPEED;

#ifdef DO_ALL_SPRITES
  scenegraph_write = !scenegraph_write;
  for(int s=0; s<NUM_SPRITES; s++){
  ; if(scenegraph[scenegraph_read][s].c==0b1000000000000000) continue;
    #define SPRITE_SPEED 16
    scenegraph[scenegraph_write][s].x=scenegraph[scenegraph_read][s].x+1+(NUM_SPRITES-s)/SPRITE_SPEED;
    scenegraph[scenegraph_write][s].y=scenegraph[scenegraph_read][s].y+1+(NUM_SPRITES-s)/SPRITE_SPEED;
    scenegraph[scenegraph_write][s].w=scenegraph[scenegraph_read][s].w+1+(NUM_SPRITES-s)/SPRITE_SPEED;
    scenegraph[scenegraph_write][s].h=scenegraph[scenegraph_read][s].h+1+(NUM_SPRITES-s)/SPRITE_SPEED;
    if(scenegraph[scenegraph_write][s].x > H_RESOLUTION) scenegraph[scenegraph_write][s].x=0;
    if(scenegraph[scenegraph_write][s].y > V_RESOLUTION) scenegraph[scenegraph_write][s].y=0;
    if(scenegraph[scenegraph_write][s].w > 400)          scenegraph[scenegraph_write][s].w=100;
    if(scenegraph[scenegraph_write][s].h > 400)          scenegraph[scenegraph_write][s].h=100;
  }
#endif

#ifdef DO_FRAME_TIME
  static int64_t ltus=0;
  int64_t ctus=time_us();
  int64_t frame_time=ctus-ltus;
  ltus=ctus;
  static int64_t ft=0;
  if(abs((int32_t)(frame_time - ft)) > 50 && frame_time < 600000){
    ft=frame_time;
    log_write("%.1fHz (%.3lluus)\n", 1e6/frame_time, frame_time);
  }
#endif
  uint64_t ct=time_ms();
  static uint64_t lt=0;
  if(lt) loop_time=(uint32_t)(ct-lt);
  lt=ct;

  if(g2d_pending()){
    onn_run_evaluators(useruid, (void*)USER_EVENT_TOUCH);
  }
}

#ifdef  DO_G2D
// #define G2D_BUFFER_SIZE (240 * 320)
extern uint16_t g2d_buffer[];
#endif

#define Y_OFFSET 20

void __not_in_flash_func(ont_hx_scanline)(uint16_t* buf, uint16_t* puf, uint16_t scan_y, bool free_time){
    if(scan_y <= Y_OFFSET) return;
#ifdef DO_WALLPAPER
    dma_memset16(buf,        0x1111, H_RESOLUTION, DMA_CH_READ, true);
 // memset(      buf, (uint8_t)0x11, H_RESOLUTION*2);
#endif // DO_WALLPAPER
#ifdef DO_G2D
    if(scan_y < (320+Y_OFFSET)){
      void* g2d_addr = (g2d_buffer + ((scan_y-Y_OFFSET) * 240));
      dma_memcpy16(buf+640, g2d_addr, 240, DMA_CH_READ, true);
   // memcpy(      buf+640, g2d_addr, 240*2);
    }
#endif
    for(int s=0; s < NUM_SPRITES; s++){

      sprite sp = scenegraph[scenegraph_read][s];
      uint16_t sx=sp.x; uint16_t sy=sp.y;
       int16_t sw=sp.w; uint16_t sh=sp.h;
      uint16_t sc=sp.c;

      bool pixel_doubling = sw < 0;
      if(pixel_doubling){
        sw = -sw*2;
        sh =  sh*2;
      }

    ; if(scan_y < sy || scan_y >= sy + sh) continue;
    ; if(sx >= H_RESOLUTION) continue;

      if(sx+sw >= H_RESOLUTION) sw=H_RESOLUTION-sx;

      if(sc & 0b1000000000000000){
#ifdef DO_IMAGE_PANEL
        bool is_even = !(scan_y % 2);
        if(!DO_INTERLACING || even_lines == is_even){

          int32_t image_line = ((uint32_t)scan_y - sy + yoff);
          if(pixel_doubling) image_line/=2;
          image_line%=V_RESOLUTION;
          void* src_addr = (psram_buffer + (image_line * H_RESOLUTION));
#ifdef DO_TIME_PSRAM
          #define PSRAM_TIME_RATE 49999
          static uint64_t lc=0;
          int64_t s=0;
          lc++;
          if(lc % PSRAM_TIME_RATE == 0){
            s=time_us();
          }
#endif
          if(!pixel_doubling){
         // dma_memcpy16(buf+sx, src_addr, sw, DMA_CH_READ, true);
            memcpy(buf+sx, src_addr, sw*2);
          } else {
            for(int16_t i=0; i<sw/2; i++){
              *(buf+sx+i*2+0)=*((uint16_t*)src_addr+i);
              *(buf+sx+i*2+1)=*((uint16_t*)src_addr+i);
            }
          }
#ifdef DO_TIME_PSRAM
          if(lc % PSRAM_TIME_RATE == 0){
            int64_t e=time_us();
            log_write("psram cpy=%.3lluus sw=%d t/pix=%.1fns\n", e-s, sw, (e-s)*1000.0f/sw);
          }
#endif
        }else{
          uint16_t* prev_line;
          if(puf){ // REVISIT: ugh
            prev_line=puf+sx;
          }else{
            prev_line=buf+sx-H_RESOLUTION;
          }
          memcpy(buf+sx, prev_line, sw*2);
       // memset(buf+sx, 0x0, sw*2);
        }
#endif
      } else {
#ifdef DO_ALL_SPRITES
        dma_memset16(buf+sx, sc, sw, DMA_CH_READ, true);
     // memset(      buf+sx, sc, sw*2);
#endif
      }
    }
    if(free_time && sram_readable) write_next_chunk_to_psram();
}

// -----------------------------------------------------

