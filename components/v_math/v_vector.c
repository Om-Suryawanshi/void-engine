#include "v_vector.h"
#include <math.h>

vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t){ a.x + b.x, a.y + b.y, a.z + b.z };
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t){ a.x - b.x, a.y - b.y, a.z - b.z };
}

vec3_t vec3_mul(vec3_t v, fix16_t scalar) {
    return (vec3_t){
        f16_mul(v.x, scalar),
        f16_mul(v.y, scalar),
        f16_mul(v.z, scalar)
    };
}

vec3_t vec3_div(vec3_t v, fix16_t scalar) {
    if (scalar == 0) return v;
    return (vec3_t){
        f16_div(v.x, scalar),
        f16_div(v.y, scalar),
        f16_div(v.z, scalar)
    };
}

fix16_t vec3_dot(vec3_t a, vec3_t b) {
    return f16_add(
        f16_add(f16_mul(a.x, b.x), f16_mul(a.y, b.y)),
        f16_mul(a.z, b.z)
    );
}

fix16_t vec3_length(vec3_t v) {
    float x = F16_TO_FLT(v.x);
    float y = F16_TO_FLT(v.y);
    float z = F16_TO_FLT(v.z);
    return FLT_TO_F16(sqrtf(x*x + y*y + z*z));
}

vec3_t vec3_normalize(vec3_t v) {
    fix16_t len = vec3_length(v);
    if (len == 0) return v;
    return vec3_div(v, len);
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    return (vec3_t){
        f16_sub(f16_mul(a.y, b.z), f16_mul(a.z, b.y)),
        f16_sub(f16_mul(a.z, b.x), f16_mul(a.x, b.z)),
        f16_sub(f16_mul(a.x, b.y), f16_mul(a.y, b.x))
    };
}

vec3_t vec3_normal(vec3_t a, vec3_t b, vec3_t c)
{
  vec3_t vector_a = vec3_sub(b, a);
  vec3_t vector_b = vec3_sub(c, a);
  vec3_t cross = vec3_cross(vector_a, vector_b);
  return vec3_normalize(cross);
}
