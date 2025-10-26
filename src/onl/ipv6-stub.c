#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <onx/log.h>
#include <onx/ipv6.h>

extern const list* onp_ipv6_groups;

bool ipv6_init(channel_recv_cb cb){
  return true;
}

uint16_t ipv6_read(char* group, char* buf, uint16_t len){
  return 0;
}

uint16_t ipv6_write(char* group, char* buf, uint16_t len){
  return 0;
}

