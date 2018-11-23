// use #include to push for faster code
#ifndef _MAPM_CLZ_C_
#define _MAPM_CLZ_C_

static int clz(unsigned x)       // leading zeroes of x
{
#if defined(__GNUC__) && defined(__i386)
  if (x == 0) return 32;
  __asm__("bsrl %1, %0" : "=r" (x) : "r" (x));
  return 31 - x;
#else
  #define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
  static const char LogTable[256] = {
    32,31,30,30,29,29,29,29,28,28,28,28,28,28,28,28,
    LT(27),LT(26),LT(26),LT(25),LT(25),LT(25),LT(25),
    LT(24),LT(24),LT(24),LT(24),LT(24),LT(24),LT(24),LT(24)
  };
  if (x & 0xFF000000) return LogTable[x >> 24] - 24;
  if (x & 0x00FF0000) return LogTable[x >> 16] - 16;
  if (x & 0x0000FF00) return LogTable[x >> 8] - 8;
  return LogTable[x];
#endif
}

#ifndef NO_CLZLL
static int clzll(unsigned long long x)
{
  return (x & ~0xffffffffULL) ? clz(x>>32) : 32 + clz(x);
}
#endif
#endif
