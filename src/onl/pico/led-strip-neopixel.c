/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>

#include <hardware/pio.h>
#include <hardware/clocks.h>

#include <onx/log.h>
#include <onx/led-strip.h>

#include <led-strip-neopixel.pio.h>

#define LED_STRIP_COUNT 1

static const uint16_t num_leds=LED_STRIP_COUNT;

static uint8_t led_strip_array[LED_STRIP_COUNT][3];

static PIO  pio;
static uint sm;
static uint offset;

void led_strip_init(){

    #define wtf pio_claim_free_sm_and_add_program_for_gpio_range
    wtf(&ws2812_program, &pio, &sm, &offset, PICO_DEFAULT_WS2812_PIN, 1, true);

    // IS_RGBW false = 3 bytes per pixel (RGB)  and urgb_u32()
    // IS_RGBW true  = 4 bytes per pixel (RGBW) and urgbw_u32()
    #define IS_RGBW false
    ws2812_program_init(pio, sm, offset, PICO_DEFAULT_WS2812_PIN, 800000, IS_RGBW);
}

void led_strip_end(){
  pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}

void led_strip_fill_hsv(colours_hsv hsv){
  colours_rgb rgb = colours_hsv_to_rgb(hsv);
  led_strip_fill_rgb(rgb);
}

void led_strip_fill_rgb(colours_rgb rgb){
  for(uint16_t i=0; i<num_leds; i++){
    led_strip_array[i][0]=rgb.r;
    led_strip_array[i][1]=rgb.g;
    led_strip_array[i][2]=rgb.b;
  }
}

void led_strip_fill_col(char* colour){
  led_strip_fill_rgb(colours_parse_string(colour));
}

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return (((uint32_t)r) << 16) |
           (((uint32_t)g) << 24) |
           (((uint32_t)b) <<  8) ;
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return (((uint32_t)r) <<  8) |
           (((uint32_t)g) << 16) |
           (((uint32_t)w) << 24) |
           (((uint32_t)b)      ) ;
}

void led_strip_show(){

  uint8_t r = led_strip_array[0][0];
  uint8_t g = led_strip_array[0][1];
  uint8_t b = led_strip_array[0][2];

  put_pixel(pio, sm, urgb_u32(r,g,b));
}





