
#include <stdbool.h>

#include <onx/serial.h>
#include <onx/log.h>

void serial_cb(bool connect, char* tty){

  if(connect) return;

  static char chars[256];
  int16_t r=serial_read(chars, 256);

  if(r > 1 && chars[0]=='#') log_char_recvd(chars[1]); // call up to onl/log.c
}

bool log_arch_init(){  // call down from onl/log.c
  if(log_to_std){
    serial_init(0,0,serial_cb); // overridden later if onp_channels ~= "serial";
    serial_ready_state();
  }
  return true;
}

bool log_arch_loop(){  // call down from onl/log.c
  if(log_to_std){
    serial_loop();
  }
  return true;
}

bool log_arch_connected(){  // call down from onl/log.c
  if(log_to_std){
    return serial_connected();
  }
  return false;
}


