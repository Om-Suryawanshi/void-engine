#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "driver/spi_master.h"

#define V_BLACK   0x0000
#define V_BLUE    0x001F
#define V_RED     0xF800
#define V_GREEN   0x07E0
#define V_CYAN    0x07FF
#define V_MAGENTA 0xF81F
#define V_YELLOW  0xFFE0
#define V_WHITE   0xFFFF

void display_init(void);

void display_fill(uint16_t color);

void display_draw_pixel(int x, int y, uint16_t color);

#endif
