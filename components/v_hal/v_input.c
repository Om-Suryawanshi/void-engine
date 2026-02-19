#include "v_input.h"
#include "v_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static volatile uint8_t input_state = 0;

#define PIN_A     BUTTON_1
#define PIN_B     BUTTON_2
#define PIN_UP    BUTTON_3
#define PIN_DOWN  BUTTON_4
#define PIN_LEFT  BUTTON_5
#define PIN_RIGHT BUTTON_6

void input_task(void *pvParameter)
{
  while(1)
  {
    uint8_t current_state = 0;

    if(!gpio_get_level(PIN_UP)) current_state |= INPUT_UP;
    if(!gpio_get_level(PIN_DOWN)) current_state |= INPUT_DOWN;
    if(!gpio_get_level(PIN_LEFT)) current_state |= INPUT_LEFT;
    if(!gpio_get_level(PIN_RIGHT)) current_state |= INPUT_RIGHT;
    if(!gpio_get_level(PIN_A)) current_state |= INPUT_A;
    if(!gpio_get_level(PIN_B)) current_state |= INPUT_B;

    input_state = current_state;

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void input_init(void)
{
  gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_INPUT,
    .pull_down_en = 0,
    .pull_up_en = 1,
    .pin_bit_mask = (1ULL << PIN_UP) | (1ULL << PIN_DOWN) |
                    (1ULL << PIN_LEFT) | (1ULL << PIN_RIGHT) |
                    (1ULL << PIN_A) | (1ULL << PIN_B)
  };

  gpio_config(&io_conf);

  xTaskCreate(input_task, "InputTask", 2048, NULL, 5, NULL);
}

uint8_t input_get(void)
{
  return input_state;
}
