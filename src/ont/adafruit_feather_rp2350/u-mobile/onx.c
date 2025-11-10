
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

const bool  onp_log         = false;
const char* onp_channels    = "radio";
const char* onp_ipv6_groups = 0;
const char* onp_radio_bands = 0;

const char* onn_test_uid_prefix = 0;

// -----------------------------------------------------

#include <mountains_800x480_rgb565.h>

#define NO_MOUNTAINS     // DO_MOUNTAINS

extern volatile int64_t frame_time;

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

// ------------------------------------------------------------------------

volatile bool scenegraph_write=false;

#define scenegraph_read (!scenegraph_write)

typedef struct sprite {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint16_t c;
} sprite;

#define NUM_SPRITES 12
static sprite scenegraph[2][NUM_SPRITES] = {
 {
  { 250, 200,300,200, 0b0111111111111111 },
  { 350, 300, 90, 90, 0b0111100111111111 },
  { 150, 100, 30, 30, 0b0111111111100111 },
  { 250, 200, 60, 60, 0b0001111111111111 },
  { 350, 350, 90, 90, 0b0000000000000000 },
  { 100, 100, 90, 90, 0b0001110011100111 },
  { 200, 200,300,200, 0b0111111111111111 },
  { 300, 300, 90, 90, 0b0111100111111111 },
  { 100, 100, 30, 30, 0b0111111111100111 },
  { 200, 200, 60, 60, 0b0001111111111111 },
  { 300, 300, 90, 90, 0b0111111111100000 },
  { 250,  50,300,300, 0b1000000000000000 }
 },{
  { 250, 200,300,200, 0b0111111111111111 },
  { 350, 300, 90, 90, 0b0111100111111111 },
  { 150, 100, 30, 30, 0b0111111111100111 },
  { 250, 200, 60, 60, 0b0001111111111111 },
  { 350, 350, 90, 90, 0b0000000000000000 },
  { 100, 100, 90, 90, 0b0001110011100111 },
  { 200, 200,300,200, 0b0111111111111111 },
  { 300, 300, 90, 90, 0b0111100111111111 },
  { 100, 100, 30, 30, 0b0111111111100111 },
  { 200, 200, 60, 60, 0b0001111111111111 },
  { 300, 300, 90, 90, 0b0111111111100000 },
  { 250,  50,300,300, 0b1000000000000000 }
 }
};

// ------------------------------------------------------------------------

uint16_t* psram_buffer =  (uint16_t*)0x15000000;

void copy_mountain_to_psram(){
  for(int y=0; y < V_RESOLUTION; y++){
    int line_offset  = (y * H_RESOLUTION);
    for(int x=0; x < H_RESOLUTION; x++){
      uint16_t pixel;
#ifdef DO_MOUNTAINS
      pixel = mountains_800x480[x + line_offset];
      pixel = ((pixel >> 1) & 0x7fe0) | (pixel & 0x001f);
#else
      if(y<V_RESOLUTION/4) pixel = (x+line_offset) & 0x7c00;
      else
      if(y<V_RESOLUTION/2) pixel = (x+line_offset) & 0x03e0;
      else
                           pixel = (x+line_offset) & 0x001f;
#endif
      psram_buffer[x + line_offset] = pixel;
    }
  }
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

  copy_mountain_to_psram();
}

uint32_t loop_time=0;

static volatile int yoff=0;

#define NO_ALL_SPRITES // DO_ALL_SPRITES

void ont_hx_frame(){ // REVISIT: only called on frame flip - do on each loop with do_flip

  yoff+=6;

#ifdef DO_ALL_SPRITES
  scenegraph_write = !scenegraph_write;
  for(int s=0; s<NUM_SPRITES; s++){
  ; if(scenegraph[scenegraph_read][s].c==0b1000000000000000) continue;
    scenegraph[scenegraph_write][s].x=scenegraph[scenegraph_read][s].x+1+(NUM_SPRITES-s)/4;
    scenegraph[scenegraph_write][s].y=scenegraph[scenegraph_read][s].y+1+(NUM_SPRITES-s)/4;
    scenegraph[scenegraph_write][s].w=scenegraph[scenegraph_read][s].w+1+(NUM_SPRITES-s)/4;
    scenegraph[scenegraph_write][s].h=scenegraph[scenegraph_read][s].h+1+(NUM_SPRITES-s)/4;
    if(scenegraph[scenegraph_write][s].x > H_RESOLUTION) scenegraph[scenegraph_write][s].x=0;
    if(scenegraph[scenegraph_write][s].y > V_RESOLUTION) scenegraph[scenegraph_write][s].y=0;
    if(scenegraph[scenegraph_write][s].w > 400)          scenegraph[scenegraph_write][s].w=100;
    if(scenegraph[scenegraph_write][s].h > 400)          scenegraph[scenegraph_write][s].h=100;
  }
#endif

  static int64_t ft=0;
  if(abs((int32_t)(frame_time - ft)) > 40 && frame_time < 600000){
    ft=frame_time;
    log_write("%.1fHz\n", 1000000.0f/frame_time);
  }

  uint64_t ct=time_ms();
  static uint64_t lt=0;
  if(lt) loop_time=(uint32_t)(ct-lt);
  lt=ct;

  if(g2d_pending()){
    onn_run_evaluators(useruid, (void*)USER_EVENT_TOUCH);
  }
}

// #define G2D_BUFFER_SIZE (240 * 320)
extern uint16_t g2d_buffer[];

void __not_in_flash_func(fill_line_sprites)(uint16_t* buf, uint32_t scan_y) {

    // if no wallpaper, time=4us; else PSRAM time=34us/36us
#define DIVPOINT (H_RESOLUTION*0/8)
    void* wll_addr = (psram_buffer + (scan_y * H_RESOLUTION));
    dma_memcpy16(buf,          wll_addr, DIVPOINT,              DMA_CH_READ, false);
    dma_memset16(buf+DIVPOINT, 0x672c,   H_RESOLUTION-DIVPOINT, DMA_CH_READ, false);
    if(scan_y<320){
      void* g2d_addr = (g2d_buffer + (scan_y * 240));
      dma_memcpy16(buf,        g2d_addr, 240,                   DMA_CH_READ, false);
    }
    for(int s=0; s < NUM_SPRITES; s++){

      sprite sp = scenegraph[scenegraph_read][s];
      uint16_t sx=sp.x; uint16_t sy=sp.y;
      uint16_t sw=sp.w; uint16_t sh=sp.h;
      uint16_t sc=sp.c;

    ; if(scan_y < sy || scan_y >= sy + sh) continue;
    ; if(sx >= H_RESOLUTION) continue;

      if(sx+sw >= H_RESOLUTION) sw=H_RESOLUTION-sx-1;

      if(sc & 0b1000000000000000){
#if DIVPOINT
        int yo = (scan_y - sy + yoff) % 480;
        void* src_addr = (psram_buffer + (yo * H_RESOLUTION) + 350);
        dma_memcpy16(buf+sx, src_addr, sw, DMA_CH_READ, false);
#endif
      }else{
#ifdef DO_ALL_SPRITES
        dma_memset16(buf+sx, sc,       sw, DMA_CH_READ, false);
#endif
      }
    }
}

void __not_in_flash_func(ont_hx_scanline)(uint16_t* buf, uint32_t scan_y){
  fill_line_sprites(buf, scan_y);
}

// -----------------------------------------------------

