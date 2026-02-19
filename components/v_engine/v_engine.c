#include "v_engine.h"
#include "v_display.h"
#include "v_input.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

  while(1)
  {
    if(config->on_update)
    {
      config->on_update(0x016f); // 60FPS
    }

    if(config->on_draw)
    {
      config->on_draw(current_mode);
    }

    display_draw();
    vTaskDelay(1);
  }
}
