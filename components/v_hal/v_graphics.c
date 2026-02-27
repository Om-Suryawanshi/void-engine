#include "v_graphics.h"
#include <stdlib.h>
#include <string.h>
#include "v_colors.h"


extern uint16_t *v_frameBuffer;

void gfx_clear(uint16_t color)
{
  if (!v_frameBuffer) return;
  if (color == V_BLACK)
  {
    memset(v_frameBuffer, 0, V_BUFFER_SIZE * 2);
  }
  else
  {
    uint16_t swapped = (color >> 8) | (color << 8);
    for (int i = 0; i < V_BUFFER_SIZE; i++)
    {
      v_frameBuffer[i] = swapped;
    }
  }
}

void gfx_draw_pixel(int x, int y, uint16_t color)
{
  if (x < 0 || x >= V_DISPLAY_WIDTH || y < 0 || y >= V_DISPLAY_HEIGHT) return;

  v_frameBuffer[y * V_DISPLAY_WIDTH + x] = (color >> 8) | (color << 8);
}

void gfx_draw_line(int x0, int y0, int x1, int y1, uint16_t color)
{
  int dx = abs(x1 - x0);
  int sx = x0 < x1 ? 1 : -1;

  int dy = -abs(y1 - y0);
  int sy = y0 < y1 ? 1 : -1;

  int err = dx + dy, e2;

  while(1)
  {
    gfx_draw_pixel(x0, y0, color);
    
    if (x0 == x1 && y0 == y1) break;
    
    e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x0 += sx;
    }

    if (e2 <= dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

void gfx_draw_rect(int x, int y, int w, int h, uint16_t color)
{
  gfx_draw_line(x, y, x + w - 1, y, color);
  gfx_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
  gfx_draw_line(x, y, x, y + h - 1, color);
  gfx_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void gfx_fill_rect(int x, int y, int w, int h, uint16_t color)
{
  for (int i = x; i < x + w; i++)
    for (int j = y; j < y + h; j++)
      gfx_draw_pixel(i, j, color);
}

static void swap(int *a, int *b) 
{ 
  int t = *a;
  *a = *b;
  *b = t;
}

void gfx_fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color)
{
  // y1 <- y2 <- y3
  if (y1 > y2) 
  {
    swap(&x1, &x2);
    swap(&y1, &y2);
  }

  if (y1 > y3) 
  {
    swap(&x1, &x3);
    swap(&y1, &y3);
  }

  if (y2 > y3) 
  {
    swap(&x2, &x3);
    swap(&y2, &y3);
  }

  int total_height = y3 - y1;

  if (total_height == 0) return;

  int i_start = (y1 < 0) ? -y1 : 0;
  int i_end   = (y1 + total_height > V_DISPLAY_HEIGHT) ? V_DISPLAY_HEIGHT - y1 : total_height;

  for(int i = i_start; i < i_end; i++)
  {
    int second_half = i > y2 - y1 || y2 == y1;
    int segment_height = second_half ? y3 - y2 : y2 - y1;
    
    if (segment_height == 0) 
      continue;

    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? y2 - y1 : 0)) / segment_height;

    int A_x = x1 + (x3 - x1) * alpha;
    int B_x = second_half ? x2 + (x3 - x2) * beta : x1 + (x2 - x1) * beta;

    if (A_x > B_x) swap(&A_x, &B_x);

    if (A_x < 0) A_x = 0;
    if (B_x >= V_DISPLAY_WIDTH) B_x = V_DISPLAY_WIDTH - 1;

    for(int j = A_x; j <= B_x; j++)
    {
      gfx_draw_pixel(j, y1 + i, color);
    }
  }
}
