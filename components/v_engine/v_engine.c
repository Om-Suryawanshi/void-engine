#include "v_engine.h"
#include "v_display.h"
#include "v_input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static render_mode_t current_mode = RENDER_WIRE;

void engine_set_mode(render_mode_t mode)
{
  current_mode = mode;
}

void engine_start(game_config_t *config)
{
  display_init();
  input_init();

  if(config->on_load) config->on_load();

  int64_t last_time = esp_timer_get_time();

  while(1)
  {
    int64_t current_time = esp_timer_get_time();
    float dt = (float)(current_time - last_time) / 1000000.0f;
    last_time = current_time;

    if(dt > 0.1f)
      dt = 0.1f;

    if(config->on_update)
    {
      config->on_update(dt); // 60FPS
    }

    if(config->on_draw)
    {
      config->on_draw(current_mode);
    }

    display_draw();
    vTaskDelay(1);
  }
}
