#ifndef V_FIXED_H
#define V_FIXED_H

#include <stdint.h>

typedef int32_t fix16_t;

#define F16_SHIFT 16
#define F16_ONE   (1 << F16_SHIFT) // 65536
#define F16_HALF  (1 << (F16_SHIFT-1)) // 32768
#define F16_ZERO  0
        

#define FLT_TO_F16(x) ((fix16_t)((x) * 65536.0f))
#define INT_TO_F16(x) ((fix16_t)((x) << F16_SHIFT))
#define F16_TO_INT(x) ((x) >> F16_SHIFT)
#define F16_TO_FLT(x) ((float)(x) / 65536.0f)


static inline fix16_t f16_add(fix16_t a, fix16_t b) { return a + b; }

static inline fix16_t f16_sub(fix16_t a, fix16_t b) { return a - b; }

static inline fix16_t f16_mul(fix16_t a, fix16_t b) 
{
  // (A * B) >> 16
  return (fix16_t)(((int64_t)a * b) >> F16_SHIFT);
}

static inline fix16_t f16_div(fix16_t a, fix16_t b)
{
  // (A << 16) / B
  return (fix16_t)(((int64_t)a << F16_SHIFT) / b);
}

fix16_t v_sin(fix16_t theta);
fix16_t v_cos(fix16_t theta);

#endif
