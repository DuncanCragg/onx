#ifndef __DSI__H
#define __DSI__H

void* dsi_init();
void  dsi_draw_bitmap(void* panel, int x_start, int y_start, int x_end, int y_end, const void* buf);

#endif
