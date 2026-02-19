#ifndef V_VECTOR_H
#define V_VECTOR_H

#include "v_fixed.h"


// Scrren Cords
typedef struct{
  fix16_t x, y;
}vec2_t;


// World Cords
typedef struct{
  fix16_t x, y, z;
}vec3_t;

vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_mul(vec3_t v, fix16_t scalar);
vec3_t vec3_div(vec3_t v, fix16_t scalar);

fix16_t vec3_dot(vec3_t a, vec3_t b); // Dot Product
fix16_t vec3_length(vec3_t v); 
vec3_t vec3_normalize(vec3_t v);
vec3_t vec3_cross(vec3_t a, vec3_t b);
vec3_t vec3_normal(vec3_t a, vec3_t b, vec3_t c);


#endif
