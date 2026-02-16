#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "v_graphics.h"
#include "v_display.h"
#include "v_config.h"
#include "v_colors.h"
#include "v_input.h"
#include "v_fixed.h"
#include "v_vector.h"
#include "v_matrix.h"

static const char *TAG = "VOID_ENGINE";

void app_main(void)
{
    ESP_LOGI(TAG, "Init...");
    display_init();
    input_init();

    int px = 64;
    int py = 80;
    int speed = 2;

    vec3_t v1 = { INT_TO_F16(10), INT_TO_F16(20), INT_TO_F16(30) };
    vec3_t v2 = { INT_TO_F16(5),  INT_TO_F16(5),  INT_TO_F16(5) };

    vec3_t result = vec3_add(v1, v2);

    // We should see X=15, Y=25, Z=35
    ESP_LOGI("MATH", "Result: X=%d Y=%d Z=%d", (int)F16_TO_INT(result.x), (int)F16_TO_INT(result.y), (int)F16_TO_INT(result.z));

    // 1. Create a point at (10, 0, 0)
    vec3_t p = { INT_TO_F16(10), 0, 0 };

// 2. Rotate 90 degrees (64/256 steps) around Z axis
    mat4_t rot = mat4_rotate_z(INT_TO_F16(64)); 

// 3. Apply rotation
    vec3_t p_rotated = mat4_mul_vec3(rot, p);

// Expected Result: X should be close to 0, Y should be close to 10
    ESP_LOGI("MATH", "Rotated: X=%d Y=%d Z=%d", 
        (int)F16_TO_INT(p_rotated.x), 
        (int)F16_TO_INT(p_rotated.y), 
        (int)F16_TO_INT(p_rotated.z)
    );

    while(1)
    {
      uint8_t keys = input_get();
      if (keys & INPUT_UP) py -= speed;
      if (keys & INPUT_DOWN) py += speed;
      if (keys & INPUT_LEFT) px += speed;
      if (keys & INPUT_RIGHT) px -= speed;

      uint16_t color = (keys & INPUT_A) ? V_RED : V_YELLOW;

      if(px < 0) px = 0;
      if(px > 120) px = 120;
      if(py < 0) py = 0;
      if(py > 150) py = 150;

      gfx_clear(V_BLACK);
      gfx_fill_rect(px, py, 8, 8, color);
      display_draw();

      vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
