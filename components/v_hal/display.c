#include "display.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_MOSI 23
#define PIN_CLK  18
#define PIN_CS   5
#define PIN_DC   2
#define PIN_RST  4

#define LCD_WIDTH 128
#define LCD_HEIGHT 160
#define OFFSET_X 0
#define OFFSET_Y 0

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
  gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
  gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);

  spi_bus_config_t buscfg = {
    .mosi_io_num = PIN_MOSI,
    .sclk_io_num = PIN_CLK,
    .miso_io_num = -1,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2 + 8
  };

  spi_device_interface_config_t devcfg = {
    .clock_speed_hz = 26 * 1000 * 1000,
    .mode = 0,
    .spics_io_num = PIN_CS,
    .queue_size = 7,
    .flags = SPI_DEVICE_HALFDUPLEX,
  };

  ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
  ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &devcfg, &spi));

  gpio_set_level(PIN_RST, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(PIN_RST, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  lcd_cmd(0x01);
  vTaskDelay(150 / portTICK_PERIOD_MS);
  lcd_cmd(0x11);
  vTaskDelay(150 / portTICK_PERIOD_MS);

  lcd_cmd(0x3A);
  uint8_t colmod[] = {0x05};
  lcd_data(colmod, 1);

  lcd_cmd(0x36);
  uint8_t madctl[] = {0xC8};
  lcd_data(madctl, 1);

  lcd_cmd(0x21);
  lcd_cmd(0x29);
}

void display_fill(uint16_t color)
{
  uint16_t x_start = OFFSET_X;
  uint16_t x_end = OFFSET_X + LCD_WIDTH - 1;
  uint16_t y_start = OFFSET_Y;
  uint16_t y_end = OFFSET_Y + LCD_HEIGHT - 1;

  uint8_t data[4];

  lcd_cmd(0x2A);
  data[0] = x_start >> 8;
  data[1] = x_start & 0xFF;
  data[2] = x_end >> 8;
  data[3] = x_end & 0xFF;
  lcd_data(data, 4);

  lcd_cmd(0x2B);
  data[0] = y_start >> 8;
  data[1] = y_start & 0xFF;
  data[2] = y_end >> 8;
  data[3] = y_end & 0xFF;
  lcd_data(data, 4);

  lcd_cmd(0x2C);
  const int lines_per_chunk = 20;
  const int chunk_size = LCD_WIDTH * lines_per_chunk;

  uint16_t *chunk_buffer = heap_caps_malloc(chunk_size * sizeof(uint16_t), MALLOC_CAP_DMA);

  if (!chunk_buffer)
  {
    ESP_LOGE("V_DISPLAY", "FAILED TO ALLOC DMA BUFFER");
    return;
  }
  
  uint16_t swapped_color = (color >> 8) | (color << 8);
  for (int i = 0; i < chunk_size; i++)
  {
    chunk_buffer[i] = swapped_color;
  }

  gpio_set_level(PIN_DC, 1);

  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = chunk_size * 16;
  t.tx_buffer = chunk_buffer;

  for(int y = 0; y < LCD_HEIGHT; y += lines_per_chunk)
  {
    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    if(ret != ESP_OK)
    {
      ESP_LOGE("V_DISPLAY", "SPI Trasmit Error");
    }
  }

  free(chunk_buffer);

}

