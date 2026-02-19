#ifndef V_ENGINE_H
#define V_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
  RENDER_WIRE = 0,
  RENDER_SOLID,
  RENDER_BOTH
} render_mode_t;

typedef struct {
  void (*on_load)(void);
  void (*on_update)(float dt);
  void (*on_draw)(render_mode_t mode);
} game_config_t;

void engine_start(game_config_t *config);
void engine_set_mode(render_mode_t mode);

#endif
