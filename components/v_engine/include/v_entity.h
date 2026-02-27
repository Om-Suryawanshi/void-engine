#ifndef V_ENTITY_H
#define V_ENTITY_H

#include "v_mesh.h"
#include "v_vector.h"
#include <stdbool.h>

typedef struct{
  vec3_t pos;
  vec3_t rot;
  const mesh_t *mesh;
  uint16_t color;
  bool active;
} entity_t;

#endif
