#ifndef V_MATRIX_H
#define V_MATRIX_H

#include "v_vector.h"

typedef struct{
  fix16_t m[4][4];
} mat4_t;

mat4_t mat4_identity(void); // Identity Matrix

mat4_t mat4_translate(fix16_t tx, fix16_t ty, fix16_t tz); // Translation matrix or movement on x, y, z

mat4_t mat4_rotate_x(fix16_t angle);
mat4_t mat4_rotate_y(fix16_t angle);
mat4_t mat4_rotate_z(fix16_t angle);

mat4_t mat4_perspective(fix16_t fov, fix16_t aspect, fix16_t near, fix16_t far); // Camera lens creates a perspective proj

mat4_t mat4_mul(mat4_t a, mat4_t b);

vec3_t mat4_mul_vec3(mat4_t m, vec3_t v);

#endif
