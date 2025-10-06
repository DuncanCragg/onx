
#include <stdbool.h>
#include <stdlib.h>

#include "pico/rand.h"

#include <onx/random.h>
#include <onx/log.h>

static bool initialised=false;

/*
  uint32_t r32 = get_rand_32();
  uint64_t r64 = get_rand_64();
  rng_128_t r128; get_rand_128(&r128);
*/
static uint32_t get_rand() {
  if(!initialised){ log_flash(1,0,0); return 0; }
  uint32_t r = get_rand_32();
  return r;
}

uint8_t random_byte() {
  return get_rand() & 0xFF;
}

static uint32_t rand_m_w = 0xDEADBEEF;
static uint32_t rand_m_z = 0xCAFEBABE;

static void seed_rand(uint32_t seed) {
  rand_m_w = (seed&0xFFFF) | (seed<<16);
  rand_m_z = (seed&0xFFFF0000) | (seed>>16);
}

static int gen_rand() {
  rand_m_z = 36969 * (rand_m_z & 65535) + (rand_m_z >> 16);
  rand_m_w = 18000 * (rand_m_w & 65535) + (rand_m_w >> 16);
  return (int)RAND_MAX & (int)((rand_m_z << 16) + rand_m_w);
}

void random_init() {

  if(initialised) return; initialised=true;

  seed_rand(get_rand());
}

uint8_t random_ish_byte() {
  return gen_rand() & 0xFF;
}


