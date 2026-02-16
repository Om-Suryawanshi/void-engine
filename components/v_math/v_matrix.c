#include "v_matrix.h"
#include <string.h>

mat4_t mat4_identity(void)
{
  mat4_t m;
  memset(&m, 0, sizeof(mat4_t));
  m.m[0][0] = F16_ONE;
  m.m[1][1] = F16_ONE;
  m.m[2][2] = F16_ONE;
  m.m[3][3] = F16_ONE;
  return m;
}

mat4_t mat4_translate(fix16_t tx, fix16_t ty, fix16_t tz)
{
  mat4_t m = mat4_identity();
  m.m[0][3] = tx;
  m.m[1][3] = ty;
  m.m[2][3] = tx;
  return m;
}

mat4_t mat4_rotate_x(fix16_t angle)
{
  fix16_t c = v_cos(angle);
  fix16_t s = v_sin(angle);
  mat4_t m = mat4_identity();
  m.m[1][1] = c; 
  m.m[1][2] = -s;
  m.m[2][1] = s;
  m.m[2][2] = c;
  return m;
}

mat4_t mat4_rotate_z(fix16_t angle)
{
  fix16_t c = v_cos(angle);
  fix16_t s = v_sin(angle);
  mat4_t m = mat4_identity();
  m.m[0][0] = c; 
  m.m[0][1] = -s;
  m.m[1][0] = s;
  m.m[1][1] = c;
  return m;
}

mat4_t mat4_rotate_y(fix16_t angle)
{
  fix16_t c = v_cos(angle);
  fix16_t s = v_sin(angle);
  mat4_t m = mat4_identity();
  m.m[0][0] = c; 
  m.m[0][2] = s;
  m.m[2][0] = -s;
  m.m[2][2] = c;
  return m;
}


mat4_t mat4_perspective(fix16_t fov, fix16_t aspect, fix16_t near, fix16_t far)
{
  mat4_t m;
  memset(&m, 0, sizeof(m));
  // Scale factor based fov

  // Need to implement tan for this instead of sin
  fix16_t fov_scale = f16_div(F16_ONE, v_sin(f16_div(fov, INT_TO_F16(2))));
  
  m.m[0][0] = f16_div(fov_scale, aspect);
  m.m[1][1] = fov_scale;
  m.m[2][2] = f16_div(far, f16_sub(far, near));
  m.m[2][3] = f16_div(f16_mul(-far, near) , f16_sub(far, near));
  m.m[3][2] = F16_ONE;
  m.m[3][3] = 0;

  return m;
}

mat4_t mat4_mul(mat4_t a, mat4_t b)
{
  mat4_t m;
  for(int r = 0; r < 4; r++)
  {
    for(int c = 0; c < 4; c++)
    {
      m.m[r][c] = f16_add(
                f16_add(f16_mul(a.m[r][0], b.m[0][c]), f16_mul(a.m[r][1], b.m[1][c])),
                f16_add(f16_mul(a.m[r][2], b.m[2][c]), f16_mul(a.m[r][3], b.m[3][c]))
            );
    }
  }
  return m;
}

vec3_t mat4_mul_vec3(mat4_t m, vec3_t v) 
{
    vec3_t res;
    res.x = f16_add(f16_add(f16_mul(m.m[0][0], v.x), f16_mul(m.m[0][1], v.y)), f16_add(f16_mul(m.m[0][2], v.z), m.m[0][3]));
    res.y = f16_add(f16_add(f16_mul(m.m[1][0], v.x), f16_mul(m.m[1][1], v.y)), f16_add(f16_mul(m.m[1][2], v.z), m.m[1][3]));
    res.z = f16_add(f16_add(f16_mul(m.m[2][0], v.x), f16_mul(m.m[2][1], v.y)), f16_add(f16_mul(m.m[2][2], v.z), m.m[2][3]));
    
    // Perspective Divide (The W component)
    fix16_t w = f16_add(f16_add(f16_mul(m.m[3][0], v.x), f16_mul(m.m[3][1], v.y)), f16_add(f16_mul(m.m[3][2], v.z), m.m[3][3]));

    if (w != 0 && w != F16_ONE) 
    {
        res.x = f16_div(res.x, w);
        res.y = f16_div(res.y, w);
        res.z = f16_div(res.z, w);
    }
    return res;
}
