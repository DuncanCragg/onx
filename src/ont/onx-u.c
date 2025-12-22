
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <onx/log.h>
#include <onx/gpio.h>
#include <onx/items.h>
#include <onx/io.h>

#include <onn.h>
#include <onr.h>
#include <ont.h>

#include <user-2d.h>

#include <g2d.h>

#include <io-evaluators.h>

// -----------------------------------------------------

extern uint16_t screen_width;  // from hstx, etc
extern uint16_t screen_height;
extern uint16_t g2d_width;     // from g2d-buffer
extern uint16_t g2d_height;

uint16_t g2d_x_pos=0;
uint16_t g2d_y_pos=0;

// -----------------------------------------------------

char* useruid;
char* homeuid;
char* inventoryuid;

static char* batteryuid;
static char* touchuid;
static char* buttonuid;
static char* clockuid;
static char* aboutuid;

object* user;
object* responses;

volatile bool     button_pressed=false;

volatile uint8_t  pending_user_event;
volatile uint32_t pending_user_event_time;

// ------------------------------------------------------------------------

static void every_second(void*){
  onn_run_evaluators(clockuid, 0);
  onn_run_evaluators(aboutuid, 0);
}

static void every_10s(void*){
  onn_run_evaluators(batteryuid, 0);
}

// ------------------------------------------------------------------------

static bool touch_down=false;

static void io_cb() {

  int16_t touch_x = ((int16_t)io.touch_x)-g2d_x_pos;
  int16_t touch_y = ((int16_t)io.touch_y)-g2d_y_pos;
  if(touch_x<0) touch_x=0; if(touch_x>g2d_width)  touch_x=g2d_width;
  if(touch_y<0) touch_y=0; if(touch_y>g2d_height) touch_y=g2d_height;

  if(io.touched){
    touch_down = true;
    g2d_touch_event(true, touch_x, touch_y);
  }
  else
  if(touch_down){ // you can get >1 touch up event so reduce to just one
    touch_down=false;
    g2d_touch_event(false, touch_x, touch_y);
  }

  onn_run_evaluators(touchuid, 0); // reads global io so don't need to pass here

  // simulate physical back button with bottom-left of screen
  if(io.touched && !button_pressed){
    #define BACK_BUTTON_SIZE 200
    if(io.touch_x < BACK_BUTTON_SIZE && io.touch_y > screen_height-BACK_BUTTON_SIZE){
      button_pressed=true;
      onn_run_evaluators(buttonuid, (void*)button_pressed);
      onn_run_evaluators(useruid, (void*)USER_EVENT_BUTTON);
    }
  }
  else
  if(!io.touched && button_pressed){
    button_pressed = false;
    onn_run_evaluators(buttonuid, (void*)button_pressed);
    onn_run_evaluators(useruid, (void*)USER_EVENT_BUTTON);
  }
}

// ------------------------------------------------------------------------

static char note_text[] = "the fat cat sat on me";

static char note_text_big[] =
  "xxxxxxxxxxxxxxxxxxx " "xxxxxxxxxxxxxxxxxx " "xxxxxxxxxxxxxxxx " "xxxxxxxxxxxxxxx "
  "xxxxxxxxxxxxxx " "xxxxxxxxxxxxx " "xxxxxxxxxxxx " "xxxxxxxxxxx " "xxxxxxxxxx "
  "xxxxxxxxx " "xxxxxxxx " "xxxxxxx " "xxxxxx " "xxxxx " "xxxx " "xxx " "xx " "x "
  "Welcome to ONX! " "and the Object Network "
  "only see data in HX " "all our data "
  "just stuff - objects " "you can link to and list " "little objects "
  "of all kinds of data " "linked together " "semantic " "on our own devices "
  "hosted by you (including you) " "sewn together " "into a shared fabric "
  "which we can all Link up " "into a shared data fabric " "mesh "
  "see objects inside other devices " "add their objects to your lists "
  "we create a data fabric "
  "from all our objects linked up " "a two-way dynamic data fabric "
  "with behaviour rules you can write yourself "
  "animated by us " "internally-animated " "programmed like a spreadsheet "
  "and animate ourselves with spreadsheet-like rules "
  "objects are live - you see them change " "you have live presence as an object yourself "
  "SS-like PL over objects as objects themselves " "can share rule object set objects "
  "----- ----- ----- ----- -----";

// -----------------------------------------------------

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
  onn_set_evaluators("eval_clock",     evaluate_clock_sync_logic, evaluate_clock_logic, 0);
  onn_set_evaluators("eval_user",      evaluate_user, 0);
  onn_set_evaluators("eval_notes",     evaluate_edit_rule, 0);
  onn_set_evaluators("eval_battery",   evaluate_battery_in, 0);
  onn_set_evaluators("eval_touch",     evaluate_touch_in, 0);
  onn_set_evaluators("eval_button",    evaluate_button_in, 0);
  onn_set_evaluators("eval_about",     evaluate_about_in, 0);

  object* battery;
  object* touch;
  object* button;
  object* bcs;
  object* oclock;
  object* watchface;
  object* home;
  object* allobjects;
  object* inventory;
  object* watch;
  object* note1;
  object* note2;
  object* notes;
  object* about;

  char* allobjectsuid;
  char* responsesuid;
  char* deviceuid;
  char* bcsuid;
  char* watchfaceuid;
  char* watchuid;
  char* note1uid;
  char* note2uid;
  char* notesuid;

  object* uid_0=onn_get_from_cache("uid-0");
  if(!uid_0){

    user      =object_new(0, "eval_user",      "user", 8);
    responses =object_new(0, "eval_default",   "user responses", 12); // REVISIT "editable"?
    battery   =object_new(0, "eval_battery",   "battery", 4);
    touch     =object_new(0, "eval_touch",     "touch", 6);
    button    =object_new(0, "eval_button",    "button", 4);
    bcs       =object_new(0, "eval_editable",  "bcs editable", 5);
    oclock    =object_new(0, "eval_clock",     "clock event", 12);
    watchface =object_new(0, "eval_editable",  "watchface editable", 9);
    home      =object_new(0, "eval_editable",  "list editable", 4);
    allobjects=object_new(0, "eval_editable",  "list editable", 4);
    inventory =object_new(0, "eval_editable",  "list editable", 4);
    watch     =object_new(0, "eval_default",   "watch", 8);
    note1     =object_new(0, "eval_notes",     "text editable", 4);
    note2     =object_new(0, "eval_notes",     "text editable", 4);
    notes     =object_new(0, "eval_notes",     "text list editable", 4);
    about     =object_new(0, "eval_about",     "about", 4);

    deviceuid    =object_property(onn_device_object, "UID");
    useruid      =object_property(user, "UID");
    responsesuid =object_property(responses, "UID");
    batteryuid   =object_property(battery, "UID");
    touchuid     =object_property(touch, "UID");
    buttonuid    =object_property(button, "UID");
    bcsuid       =object_property(bcs, "UID");
    clockuid     =object_property(oclock, "UID");
    watchfaceuid =object_property(watchface, "UID");
    homeuid      =object_property(home, "UID");
    allobjectsuid=object_property(allobjects, "UID");
    inventoryuid =object_property(inventory, "UID");
    watchuid     =object_property(watch, "UID");
    note1uid     =object_property(note1, "UID");
    note2uid     =object_property(note2, "UID");
    notesuid     =object_property(notes, "UID");
    aboutuid     =object_property(about, "UID");

    object_property_set(user, "responses", responsesuid);
    object_property_set(user, "inventory", inventoryuid);

    object_property_set(bcs, "brightness", "128");
    object_property_set(bcs, "colour",     "128");
    object_property_set(bcs, "softness",   "128");

    object_set_persist(oclock, "none");
    object_property_set(oclock, "title", "ONX Clock");
    object_property_set(oclock, "ts", "%unknown");
    object_property_set(oclock, "tz", "%unknown");
//  object_property_set(oclock, "device", deviceuid); // REVISIT: peer discovery isn't great

    object_property_set(watchface, "clock", clockuid);
    object_property_set(watchface, "ampm-24hr", "ampm");

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
    object_property_add(home, "list", bcsuid);
    object_property_add(home, "list", allobjectsuid);
    object_property_add(home, "list", notesuid);

    object_property_set(allobjects, "title", "All objects");
    object_property_add(allobjects, "list", deviceuid);
    object_property_add(allobjects, "list", homeuid);
    object_property_add(allobjects, "list", inventoryuid);
    object_property_add(allobjects, "list", notesuid);
    object_property_add(allobjects, "list", aboutuid);
    object_property_add(allobjects, "list", watchuid);
    object_property_add(allobjects, "list", clockuid);
    object_property_add(allobjects, "list", batteryuid);
    object_property_add(allobjects, "list", touchuid);
    object_property_add(allobjects, "list", buttonuid);
    object_property_add(allobjects, "list", watchfaceuid);
    object_property_add(allobjects, "list", useruid);
    object_property_add(allobjects, "list", note1uid);
    object_property_add(allobjects, "list", note2uid);
    object_property_add(allobjects, "list", responsesuid);

    object_property_set(inventory, "title", "Inventory");

    object_property_set(watch, "battery",   batteryuid);
    object_property_set(watch, "watchface", watchfaceuid);

    object_property_set(user, "viewing", watchuid);

    object_property_set(onn_device_object, "name", "MobCon5");
    object_property_add(onn_device_object, "user", useruid);
    object_property_add(onn_device_object, "io",   batteryuid);
    object_property_add(onn_device_object, "io",   touchuid);
    object_property_add(onn_device_object, "io",   buttonuid);
    object_property_add(onn_device_object, "io",   clockuid);

    object_set_persist(battery, "none");
    object_set_persist(touch,   "none");
    object_set_persist(button,  "none");
    object_set_persist(about,   "none");

    uid_0=object_new("uid-0", 0, "config", 10);
    object_property_set(uid_0, "user", useruid);

  } else {

    useruid = object_property(uid_0, "user");

    user = onn_get_from_cache(useruid);
  }
  onn_run_evaluators(batteryuid, 0);
  onn_run_evaluators(clockuid, 0);
  onn_run_evaluators(aboutuid, 0);
  onn_run_evaluators(useruid, (void*)USER_EVENT_INITIAL);
}

// ------------------------------------------------------------------------

void onx_u_init(){

  gpio_init();

  io_init(io_cb);

  g2d_init();
  g2d_x_pos=100;
  g2d_y_pos=100;

  init_onx();

  time_tick(every_second, 0,  1000);
  time_tick(every_10s,    0, 10000);
}

uint32_t loop_time=0;
 
void onx_u_loop(){

  uint64_t ct=time_ms();
  static uint64_t lt=0;
  if(lt) loop_time=(uint32_t)(ct-lt);
  lt=ct;

  if(g2d_pending()){
    onn_run_evaluators(useruid, (void*)USER_EVENT_TOUCH);
  }
  if(gfx_log_buffer && list_size(gfx_log_buffer)){
    onn_run_evaluators(useruid, (void*)USER_EVENT_LOG);
  }
  if(pending_user_event_time && ct > pending_user_event_time){
    if(pending_user_event & USER_EVENT_NONE)   onn_run_evaluators(useruid, (void*)USER_EVENT_NONE_AL);
    if(pending_user_event & USER_EVENT_BUTTON) onn_run_evaluators(useruid, (void*)USER_EVENT_BUTTON);
    if(pending_user_event & USER_EVENT_TOUCH)  onn_run_evaluators(useruid, (void*)USER_EVENT_TOUCH);
    if(pending_user_event & USER_EVENT_LOG)    onn_run_evaluators(useruid, (void*)USER_EVENT_LOG);
    pending_user_event=0;
    pending_user_event_time=0;
  }
}

