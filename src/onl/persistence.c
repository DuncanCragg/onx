
// NOT sure what this is for: it's just a thin wrapper around database
// move code to ONN?

#if defined(BOARD_YOU_FLASH)
#define FLASH_NOT_FAKE
#endif

#include <inttypes.h>
#define FMT_UINT32 PRIu32

#include <onx/items.h>
#include <onx/lib.h>
#include <onx/boot.h>
#include <onx/mem.h>
#include <onx/log.h>
#if defined(FLASH_NOT_FAKE)
#include <onx/qspi-flash.h>
#include <onx/database.h>
#endif

#include <persistence.h>

#define MAX_OBJECTS 64

#if defined(FLASH_NOT_FAKE)

static database_storage* db = 0;

#define FLASH_DB_SECTOR_SIZE  4096
#define FLASH_DB_SECTOR_COUNT  512 // REVISIT: we only do 2Mb flash!
#define FLASH_SIZE (FLASH_DB_SECTOR_SIZE * FLASH_DB_SECTOR_COUNT)

static void flash_db_init(database_storage* db){
}

static void flash_db_erase(database_storage* db, uint32_t address, uint16_t size, void (*cb)()){
  char* err = qspi_flash_erase(address, QSPI_FLASH_ERASE_LEN_4KB, 0);
  if(err){ log_write("flash_db_erase error: %s", err); return; }
}

static void flash_db_write(database_storage* db, uint32_t address, uint8_t* buf, uint16_t size, void (*cb)()){
  char* err = qspi_flash_write(address, buf, size, 0);
  if(err){ log_write("flash_db_write error: %s", err); return; }
}

static void flash_db_read(database_storage* db, uint32_t address, uint8_t* buf, uint16_t size, void (*cb)()){
  char* err = qspi_flash_read(address, buf, size, 0);
  if(decent_string((char*)buf)) log_write("flash_db_read: size=%d len=%d [%s]\n", size, strlen((char*)buf), buf);
  if(err){ log_write("flash_db_read error: %s", err); return; }
}

static void flash_db_format(database_storage* db){
  log_write("flash_db_format\n");
  for(uint16_t s=0; s < db->sector_count; s++){
    flash_db_erase(db, db->sector_size * s, db->sector_size, 0);
    database_sector_info dsi;
    dsi.erase_count = (s==0? 2: 1);
    dsi.zero_term = 0;
    flash_db_write(db, db->sector_size * s, (uint8_t*)&dsi, sizeof(database_sector_info), 0);
    boot_feed_watchdog(); // REVISIT: use the callbacks, duh
    log_write(".");
  }
  log_write("\nflash_db_format done\n");
}

static database_storage* flash_db_storage_new(){

  database_storage* db=mem_alloc(sizeof(database_storage));
  if(!db) return 0;

  (*db).sector_size  = FLASH_DB_SECTOR_SIZE;
  (*db).sector_count = FLASH_DB_SECTOR_COUNT;

  (*db).init   = flash_db_init;
  (*db).format = flash_db_format;
  (*db).erase  = flash_db_erase;
  (*db).write  = flash_db_write;
  (*db).read   = flash_db_read;

  return db;
}
#else

static properties* persistence_objects_text=0;

#endif

static list* keep_actives = 0;

list* persistence_init(properties* config){

#if defined(FLASH_NOT_FAKE)

  char allids[64];
  char* err = qspi_flash_init(allids);
  if(err){ log_write("persistence_init error: %s", err); return 0; }
  else     log_write("flash: %s\n", allids);

  db = flash_db_storage_new();

  keep_actives = database_init(db,config);
  return keep_actives;
#else
  persistence_objects_text=properties_new(MAX_OBJECTS);
  keep_actives = list_new(64);
  return 0;
#endif
}

// for testing
list* persistence_reload(){
#if defined(FLASH_NOT_FAKE)
  database_free(db);
  keep_actives = database_init(db,0);
#endif
  return keep_actives;
}

void  persistence_wipe(){
#if defined(FLASH_NOT_FAKE)
  if(db) database_wipe(db);
  else log_write("persistence_wipe but no db\n");
#else
  log_write("persistence_wipe...\n");
#endif
}

void persistence_dump(){
#if defined(FLASH_NOT_FAKE)
  if(db) database_dump(db);
  else log_write("persistence_dump but no db\n");
#else
  log_write("persistence_dump...\n");
#endif
}

char* persistence_get(char* uid){
#if defined(FLASH_NOT_FAKE)
  static char obj_text[2048];
  uint16_t s = database_get(db, uid, 0, (uint8_t*)obj_text, 2048);
  return obj_text; // REVISIT: hmmmmm
#else
  return properties_get(persistence_objects_text, uid);
#endif
}

void persistence_put(char* uid, uint32_t ver, char* text) {
#if defined(FLASH_NOT_FAKE)
  if(!text || !(*text)) return;
  bool ok=database_put(db, uid, ver, (uint8_t*)text, strlen(text)+1);
#else
  mem_freestr(properties_del(persistence_objects_text, uid));
  properties_set(persistence_objects_text, uid, mem_strdup(text));
  bool ka = strstr(text, "Cache: keep-active");
  if(ka) list_vals_set_add(keep_actives, uid);
#endif
}


