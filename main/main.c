#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "display.h"
#include "math.h"

static const char *TAG = "VOID_ENGINE";

void app_main(void)
{
  ESP_LOGI(TAG, "Init..");
  display_init();

  while(1)
  {
    display_fill(V_BLACK);

    display_fill(V_RED);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    display_fill(V_BLUE);
    vTaskDelay(100 / portTICK_PERIOD_MS);

  }
}
