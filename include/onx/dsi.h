#ifndef __DSI__H
#define __DSI__H

void* dsi_init();
void  dsi_loop();
void  dsi_draw_bitmap(void* panel, void* buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool block);

#endif
