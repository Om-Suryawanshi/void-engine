#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5
#define PIN_NUM_DC   2
#define PIN_NUM_RST  4
#define PIN_NUM_BCKL 21 

#define LCD_WIDTH  128
#define LCD_HEIGHT 160

#define OFFSET_X  0
#define OFFSET_Y  0

#define SPI_CLOCK_SPEED_HZ (26 * 1000 * 1000) 

#define CMD_SWRESET 0x01
#define CMD_SLPOUT  0x11
#define CMD_INVON   0x21 
#define CMD_DISPON  0x29
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C
#define CMD_MADCTL  0x36
#define CMD_COLMOD  0x3A

#define CMD_INVOFF 0x20

static const char *TAG = "VOID_ENGINE";
spi_device_handle_t spi;

// --- LOW LEVEL SPI ---
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       
    t.length = 8;                   
    t.tx_buffer = &cmd;             
    t.user = (void*)0;              
    
    gpio_set_level(PIN_NUM_DC, 0); 
    ret = spi_device_polling_transmit(spi, &t); 
    assert(ret == ESP_OK);
}

void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len) {
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) return;             
    memset(&t, 0, sizeof(t));       
    t.length = len * 8;             
    t.tx_buffer = data;             
    t.user = (void*)1;              
    
    gpio_set_level(PIN_NUM_DC, 1);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

// --- INITIALIZATION ---
void lcd_init(spi_device_handle_t spi) {
    // 1. Hardware Reset
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // 2. Software Reset & Wakeup
    lcd_cmd(spi, CMD_SWRESET); 
    vTaskDelay(150 / portTICK_PERIOD_MS);

    lcd_cmd(spi, CMD_SLPOUT); 
    vTaskDelay(150 / portTICK_PERIOD_MS);

    // 3. Color Mode 16-bit
    lcd_cmd(spi, CMD_COLMOD);
    uint8_t data[] = {0x05}; // 16-bit color (0x05 for ST7735, sometimes 0x55)
    lcd_data(spi, data, 1);

    // 4. Memory Access & Rotation
    lcd_cmd(spi, CMD_MADCTL);
    // 0xC0 = MY | MX (Flip X and Y) -> Matches standard "Red Tab" orientation
    // Try 0xC8 for BGR, 0xC0 for RGB.
    uint8_t madctl[] = {0xC8}; 
    lcd_data(spi, madctl, 1);
    
    lcd_cmd(spi, CMD_INVOFF); 

    // 6. Display On
    lcd_cmd(spi, CMD_DISPON);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "Display Initialized (128x160).");
}

// --- FILL SCREEN (Corrected for 128x160) ---
void lcd_fill_color(spi_device_handle_t spi, uint16_t color) {
    uint8_t data[4];

    // Apply Offsets (Handles the "Black Tab" / "Red Tab" shift)
    uint16_t x_start = OFFSET_X;
    uint16_t x_end = OFFSET_X + LCD_WIDTH - 1;
    uint16_t y_start = OFFSET_Y;
    uint16_t y_end = OFFSET_Y + LCD_HEIGHT - 1;

    // Set Column Address
    lcd_cmd(spi, CMD_CASET);
    data[0] = x_start >> 8; data[1] = x_start & 0xFF; 
    data[2] = x_end >> 8;   data[3] = x_end & 0xFF;
    lcd_data(spi, data, 4);

    // Set Row Address
    lcd_cmd(spi, CMD_RASET);
    data[0] = y_start >> 8; data[1] = y_start & 0xFF; 
    data[2] = y_end >> 8;   data[3] = y_end & 0xFF;
    lcd_data(spi, data, 4);

    // Write to RAM
    lcd_cmd(spi, CMD_RAMWR);
    
    // Allocate Buffer: 20 lines * 128 pixels * 2 bytes = 5,120 bytes
    // This is small enough for a single transaction!
    const int lines_per_chunk = 20;
    const int chunk_size = LCD_WIDTH * lines_per_chunk; 
    
    uint16_t *chunk_buffer = heap_caps_malloc(chunk_size * sizeof(uint16_t), MALLOC_CAP_DMA);
    
    // Swap bytes for SPI (Little Endian -> Big Endian)
    uint16_t swapped_color = (color >> 8) | (color << 8);
    for (int i = 0; i < chunk_size; i++) {
        chunk_buffer[i] = swapped_color;
    }

    gpio_set_level(PIN_NUM_DC, 1); // Data Mode

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = chunk_size * 16; // Bits
    t.tx_buffer = chunk_buffer;

    for (int y = 0; y < LCD_HEIGHT; y += lines_per_chunk) {
        ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
    }

    free(chunk_buffer);
}

void app_main(void)
{
    esp_err_t ret;

    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_BCKL, 1); 

    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2 + 8
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_CLOCK_SPEED_HZ, 
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,           
        .queue_size = 7,
        .flags = SPI_DEVICE_HALFDUPLEX,       
    };

    ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    lcd_init(spi);

    while (1) {
        // Red
        lcd_fill_color(spi, 0xF800);
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Green
        lcd_fill_color(spi, 0x07E0);
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Blue
        lcd_fill_color(spi, 0x001F);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
