#ifndef V_CONFIG_H
#define V_CONFIG_H


// Display
#define PIN_MOSI 23
#define PIN_CLK  18
#define PIN_CS   5
#define PIN_DC   2
#define PIN_RST  4

#define V_DISPLAY_WIDTH  128
#define V_DISPLAY_HEIGHT 160

#define V_OFFSET_X 0
#define V_OFFSET_Y 0

#define V_SPI_SPEED_HZ    (60 * 1000 * 1000) // 26 MHz
#define V_DMA_CHUNK_LINES 20                 // How many lines to send at once
#define V_RESET_DELAY_MS  100                // Delay for hardware reset
#define V_BOOT_DELAY_MS   150                // Delay for screen wakeup

#define V_BUFFER_SIZE (V_DISPLAY_WIDTH * V_DISPLAY_HEIGHT) // Total pixels

// Buttons 
#define BUTTON_1 14 
#define BUTTON_2 13 
#define BUTTON_3 25 
#define BUTTON_4 27 
#define BUTTON_5 26 
#define BUTTON_6 32 

#endif
