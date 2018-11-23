#ifndef _DTOAFAST_H_
#define _DTOAFAST_H_

#include <math.h>
#include <fenv.h>
#include <errno.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include "common.h"
#include "mapm_clz.c"
#include "dtoa-cnst.c"

#if !defined(USE_MAPM)
#define USE_MAPM        1
#endif
#if !defined(DTOA_FUNC)
#define DTOA_FUNC       extern
#endif

#ifdef __cplusplus
extern "C" {
#endif
DTOA_FUNC double strtod_fast(const char *s, char **e);
DTOA_FUNC char* dtoa_fast(double x, int digits, int *sgn, int *len, int *dec);
DTOA_FUNC char* dtoa_ifmt(char *s, int sgn, int len, int dec, char mode);
#ifdef __cplusplus
}
#endif

#ifndef DEBUG
#define DEBUG     0
#endif
#define EPRINT(...)     if (DEBUG) fprintf(stderr, __VA_ARGS__)
#define E_FMT           "0x0.%08x,%08x,%08xp%+-5d\t%s\n"
#define BIT96(s)        EPRINT(E_FMT, n2,n1,n0,bexp-BIAS,s)

#define BIAS      1021          // x = 0x.ddddd ... p(bexp-BIAS)
#define HOLE      ~98U          // accuracy of 96-bits
#define FASTPATH  (BIAS + 89)   // speedup strtod_ulp()
#define ERR       0x1p-35       // frac 35+ bits accurate
#define LAST      19            // dtoa_case last digit idx
#define EXP_MIN   -323          // below 0.1e-323 -> zero
#define EXP_MAX   +309          // above 0.9e+309 -> inf

// -D FAST_BF96: multiply only once
// max(c[0]) = 0xffbbcfe9 < 0xffffffff
// -> c[0]*n1 + 3 (x0 >> 32) under 64 bits

#define MUL_96(n2,n1,n0,bexp,i) do {\
  unsigned *c = fast_bf96[i + 351];\
  uint64_t x0 = (uint64_t)c[0] * n0;\
  uint64_t x1 = (uint64_t)c[0] * n1;\
  uint64_t x2 = (uint64_t)c[0] * n2;\
  x1 += x0 >> 32; x0 = (unsigned)x0 + (uint64_t)c[1] * n1;\
  x1 += x0 >> 32; x0 = (unsigned)x0 + (uint64_t)c[2] * n2;\
  x1 += x0 >> 32;\
  x2 += x1 >> 32; x1 = (unsigned)x1 + (uint64_t)c[1] * n2;\
  x2 += x1 >> 32;\
  bexp += (int)(i) * 108853 >> 15;\
  NORMALIZE(n2,n1,n0,bexp, x2,x1,x0, 1);\
} while(0)

// multiply upto 8 times with special constants
// no need to worry carries overflow 64 bits

#define DIV_1E73(n2,n1,n0,bexp) \
  _MUL96(n2,n1,n0,bexp,0xb4ecd5f0ULL,0x1a4aa828ULL,0x1e38aeb6ULL,-242)
#define MUL_1E60(n2,n1,n0,bexp) \
  _MUL96(n2,n1,n0,bexp,0x9f4f2726ULL,0x179a2245ULL,0x01d76242ULL,200)
#define MUL_1E25(n2,n1,n0,bexp) \
  _MUL64(n2,n1,n0,bexp,0x84595161ULL,0x401484A0ULL,84)
#define MUL_1E12(n2,n1,n0,bexp) \
  _MUL32(n2,n1,n0,bexp,0xe8d4a510ULL,40)
#define MUL_32(n2,n1,n0,bexp,i) \
  _MUL32(n2,n1,n0,bexp, fast_f32[i], fast_f32_bexp[i])

#define _MUL96(n2,n1,n0,bexp, t2,t1,t0,t_bexp) do {\
  uint64_t x0 = t2 * n0  + t1 * n1 + t0 * n2;\
  uint64_t x1 = (x0>>32) + t2 * n1 + t1 * n2;\
  uint64_t x2 = (x1>>32) + t2 * n2;\
  NORMALIZE(n2,n1,n0,bexp, x2,x1,x0,t_bexp);\
} while(0)

#define _MUL64(n2,n1,n0,bexp, t1,t0,t_bexp) do {\
  uint64_t x0 = t1 * n0  + t0 * n1;\
  uint64_t x1 = (x0>>32) + t1 * n1 + t0 * n2;\
  uint64_t x2 = (x1>>32) + t1 * n2;\
  NORMALIZE(n2,n1,n0,bexp, x2,x1,x0,t_bexp);\
} while(0)

#define _MUL32(n2,n1,n0,bexp, k,k_bexp) do {\
  uint64_t x0 = (uint64_t) k * n0;\
  uint64_t x1 = (x0>>32) + (uint64_t) k * n1;\
  uint64_t x2 = (x1>>32) + (uint64_t) k * n2;\
  NORMALIZE(n2,n1,n0,bexp, x2,x1,x0,k_bexp);\
} while(0)

#define NORMALIZE(n2,n1,n0,bexp, x2,x1,x0,size)\
if (x2 & (1ULL<<63)) {\
  n2 = (x2 >> 32); n1 = x2; n0 = x1;\
  bexp += (size);\
} else {\
  n2 = x2 >> 31;\
  n1 = ((unsigned) x1 >> 31) | ((unsigned) x2 << 1);\
  n0 = ((unsigned) x0 >> 31) | ((unsigned) x1 << 1);\
  bexp += (size) - 1;\
}
#endif
