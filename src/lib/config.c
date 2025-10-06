
#include <string.h>
#include <onx/config.h>
#include <onx/log.h>

properties* get_config(int argc, char *argv[], char* name, char* flags){

  if(argc==1){
    log_write("Usage: %s <mcast groups: ff12::1234 ff12::4321>\n", argv[0]);
    return 0;
  }

  properties* config = properties_new(32);

  char dbname[128]; snprintf(dbname, 128, "%s.ondb", name);
  properties_set(config, "db-path", value_new(dbname));

  list* chans  = list_new(2);
  list* groups = 0;

  for(int a=1; a<argc; a++){
    char* arg = argv[a];
    if(strchr(arg, ':')){
      if(!groups) groups=list_new(4);
      list_vals_add(groups, arg);
      list_vals_set_add(chans, "ipv6");
    }
  }
  if(list_size(chans))  properties_set(config, "channels",    chans);
  if(list_size(groups)) properties_set(config, "ipv6_groups", groups);

  properties_set(config, "test-uid-prefix", value_new(name));
  properties_set(config, "flags", list_vals_new_from_fixed(flags));

  return config;
}


