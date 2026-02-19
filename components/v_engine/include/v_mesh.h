#ifndef V_MESH_H
#define V_MESH_H

#include "v_vector.h"

typedef struct {
  const vec3_t *vertices;
  int num_vertices;

  const int (*faces)[3];
  int num_faces;

  const int (*edges)[2];
  int num_edges;
} mesh_t;

#endif
