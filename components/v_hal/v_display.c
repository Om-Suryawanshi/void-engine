#include "v_display.h"
#include "v_config.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"


// Command Reg
#define CMD_SWRESET 0x01 // Software Reset
#define CMD_SLPOUT  0x11 // Sleep Out
#define CMD_INVON   0x21 // Inversion On
#define CMD_INVOFF  0x20 // Inversion Off
#define CMD_DISPON  0x29 // Display On
#define CMD_CASET   0x2A // Column Address Set
#define CMD_RASET   0x2B // Row Address Set
#define CMD_RAMWR   0x2C // Memory Write
#define CMD_MADCTL  0x36 // Memory Access Control (Rotation)
#define CMD_COLMOD  0x3A // Interface Pixel Format

// Data constants
#define COLMOD_16BIT 0x05
#define MADCTL_BGR   0xC8
#define MADCTL_RGB   0xC0


uint16_t *v_frameBuffer = NULL;


static spi_device_handle_t spi;

static void lcd_cmd(const uint8_t cmd)
{
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;
  t.tx_buffer = &cmd;
  gpio_set_level(PIN_DC, 0);
  spi_device_polling_transmit(spi, &t);
}

static void lcd_data(const uint8_t *data, int len)
{
  if(len == 0) return;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = len * 8;
  t.tx_buffer = data;
  gpio_set_level(PIN_DC, 1);
  spi_device_polling_transmit(spi, &t);
}

void display_init(void)
{
  v_frameBuffer = (uint16_t*)heap_caps_calloc(V_BUFFER_SIZE, sizeof(uint16_t), MALLOC_CAP_DMA);

  if(!v_frameBuffer)
  {
    ESP_LOGE("Display.h", "Failed to allocate framebuffer");
    return;
  }

  gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);

  spi_bus_config_t buscfg = {
    .mosi_io_num = PIN_MOSI,
    .sclk_io_num = PIN_CLK,
    .miso_io_num = -1,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = V_DISPLAY_WIDTH * V_DISPLAY_HEIGHT * 2 + 8
  };

  spi_device_interface_config_t devcfg = {
    .clock_speed_hz = V_SPI_SPEED_HZ,
    .mode = 0,
    .spics_io_num = PIN_CS,
    .queue_size = 7,
    .flags = SPI_DEVICE_HALFDUPLEX,
  };

  ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
  ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &devcfg, &spi));

  gpio_set_level(PIN_RST, 0);
  vTaskDelay(V_RESET_DELAY_MS / portTICK_PERIOD_MS);
  gpio_set_level(PIN_RST, 1);
  vTaskDelay(V_RESET_DELAY_MS / portTICK_PERIOD_MS);

  lcd_cmd(CMD_SWRESET);
  vTaskDelay(V_BOOT_DELAY_MS / portTICK_PERIOD_MS);

  lcd_cmd(CMD_SLPOUT);
  vTaskDelay(V_BOOT_DELAY_MS / portTICK_PERIOD_MS);

  lcd_cmd(CMD_COLMOD);
  uint8_t colmod[] = { COLMOD_16BIT };
  lcd_data(colmod, 1);

  lcd_cmd(CMD_MADCTL);
  uint8_t madctl[] = { MADCTL_RGB };
  lcd_data(madctl, 1);

  lcd_cmd(CMD_INVOFF);
  lcd_cmd(CMD_DISPON);
}

/*
void display_set_window(int x, int y, int w, int h)
{
  uint8_t data[4];
  
  uint16_t x_start = x + V_OFFSET_X;
  uint16_t x_end = x + w - 1 + V_OFFSET_X;
  uint16_t y_start = y + V_OFFSET_Y;
  uint16_t y_end = y + h - 1 + V_OFFSET_Y;

  lcd_cmd(CMD_CASET);
  data[0] = x_start >> 8;
  data[1] = x_start & 0xFF;
  data[2] = x_end >> 8;
  data[3] = x_end & 0xFF;
  lcd_data(data, 4);

  lcd_cmd(CMD_RASET);
  data[0] = y_start >> 8;
  data[1] = y_start & 0xFF;
  data[2] = y_end >> 8;
  data[3] = y_end & 0xFF;
  lcd_data(data, 4);

  lcd_cmd(CMD_RAMWR);
}

void display_push_color(uint16_t color)
{
  uint8_t data[2] = {color >> 8, color & 0xFF};
  lcd_data(data, 2);
}

void display_push_pixels(uint16_t *colors, int count)
{
  if (count == 0) return;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = count * 16;
  t.tx_buffer = colors;

  gpio_set_level(PIN_DC, 1);
  spi_device_polling_transmit(spi, &t);
}

void display_fill_rect(int x, int y, int w, int h, uint16_t color)
{
  display_set_window(x, y, w, h);
  const int chunk_size = w * V_DMA_CHUNK_LINES;
  uint16_t *buffer = heap_caps_malloc(chunk_size * 2, MALLOC_CAP_DMA);

  uint16_t swapped = (color >> 8) | (color << 8);
  for (int i = 0; i < chunk_size; i++)
  {
    buffer[i] = swapped;
  }

  int total_pixels = w * h;
  int pixels_sent = 0;

  while(pixels_sent < total_pixels)
  {
    int pixels_to_send = total_pixels - pixels_sent;
    if (pixels_to_send > chunk_size)
    {
      pixels_to_send = chunk_size;
    }

    display_push_pixels(buffer, pixels_to_send);
    pixels_sent += pixels_to_send;
  }

  free(buffer);
}
*/

void display_draw(void)
{
  if (!v_frameBuffer) return;

  uint8_t data[4];
  lcd_cmd(CMD_CASET);
  data[0] = 0; data[1] = 0 + V_OFFSET_X;
    data[2] = (V_DISPLAY_WIDTH-1) >> 8; 
    data[3] = (V_DISPLAY_WIDTH-1) & 0xFF;
    lcd_data(data, 4);

    lcd_cmd(CMD_RASET);
    data[0] = 0; data[1] = 0 + V_OFFSET_Y;
    data[2] = (V_DISPLAY_HEIGHT-1) >> 8; 
    data[3] = (V_DISPLAY_HEIGHT-1) & 0xFF;
    lcd_data(data, 4);

    lcd_cmd(CMD_RAMWR);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = V_BUFFER_SIZE * 16; // Total bits
    t.tx_buffer = v_frameBuffer;
    
    gpio_set_level(PIN_DC, 1);
    // polling_transmit blocks until done (safe). 
    // Later we can use queue_transmit for async double-buffering!
    spi_device_polling_transmit(spi, &t);
}
