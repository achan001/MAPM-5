#include "m_apm.h"
#include "mapm-dtoa.h"
#define ULP()   M_check_ulp(r, n2, n1, bexp)

static int M_check_ulp(M_APM d, unsigned n2, unsigned n1, int bexp)
{
  if (d->m_apm_exponent >= d->m_apm_datalength && bexp <= FASTPATH)
    return 0;

  M_APM b = M_get_stack_var();  // half-way binary estimate
  n1 = ((n1>>9) + 1) >> 1;      // build rounded-up 54 bits
  m_apm_set_uint64_t(b, ((uint64_t)n2<<22) + n1);
  m_apm_ishift(b, bexp-BIAS-54);

  int sign = m_apm_compare_absolute(d, b);
  M_restore_stack(1);
  return sign;
}

static double
M_to_double (M_APM r, uint64_t top, unsigned bot, int bexp, int i)
{
  unsigned n2 = top >> 32;  // top    32 bits
  unsigned n1 = top;        // middle 32 bits
  unsigned n0 = bot;        // bottom 32 bits
  
#if FAST_BF96
  MUL_96(n2,n1,n0,bexp, i);
#else
  for(; i < 0; i += 73) DIV_1E73(n2,n1,n0,bexp);
  for(; i>=60; i -= 60) MUL_1E60(n2,n1,n0,bexp);
  for(; i>=25; i -= 25) MUL_1E25(n2,n1,n0,bexp);
  for(; i>=13; i -= 12) MUL_1E12(n2,n1,n0,bexp);
  MUL_32(n2,n1,n0,bexp, i);
#endif

  bot = n0;
  if ((unsigned) bexp >= 2046) {  // not normal number !
    if (bexp >= 2046) return HUGE_VAL;
    i = -bexp;
    bexp = 0;
    uint64_t tail = ((uint64_t) n1 << 32 | n0) >> i;
    n0 = tail |= (uint64_t) n2 << (64-i);
    n1 = tail >> 32;
    n2 = (uint64_t) n2 >> i;      // fix sub-normal
  }

  if (bot == 0) {
    if ((n1 & 0xfff) == 0x400)    // round 01000 ...
      if (n0 == 0)
        n1 += ULP() - 1;
  } else if (bot > HOLE) {
    if ((n1 & 0x7ff) == 0x3ff)    // round ?0111 ...
      if (n0 > HOLE)
        n1 += ULP() + !!(n1&0x800);
  }

  union HexDouble u;
  u.u = (uint64_t) bexp << 52;
  u.u += ((uint64_t) n2<<21) + (((n1 >> 10) + 1) >> 1);
  return u.d;
}

// Convert M_APM number to C double
//
double m_apm_to_double(M_APM r)
{
  if (r->m_apm_sign == 0) return 0.0;
  if (r->m_apm_exponent > EXP_MAX) return r->m_apm_sign * HUGE_VAL;
  if (r->m_apm_exponent < EXP_MIN) return r->m_apm_sign * 0.0;

  UCHAR *data = r->m_apm_data;
  uint64_t bot=0, top = *data++;

  int nbytes = (1 + r->m_apm_datalength) >> 1;
  int nexp   = r->m_apm_exponent;
  int i, len = (nbytes < 9) ? nbytes : 9;

  while (--len)               // build to 18 digits
    top = 100 * top + *data++;

  if (nbytes > 9) {           // build to 28 digits
    if (nbytes > 14) nbytes = 14;
    i = nbytes - 9;
    bot = top & (~0U>>2);
    top >>= 30;
    do {
      bot = 100 * bot + *data++;
      top *= 100;
    } while (--i);
    top += bot >> 30;
    i = clzll(top);
    bot = bot << 34 >> (32-i);
    top = (top << i) | (bot >> 32);
    i -= 30;
  } else
    top <<= (i = clzll(top));

  nexp -= 2 * nbytes;
  return r->m_apm_sign * M_to_double(r, top, bot, 64+BIAS-i, nexp);
}

// Convert a double into M_APM, rounded to places
// For exact conversion, set places = -1

void M_set_double(M_APM r, double d, int places)
{
  union HexDouble hd = {fabs(d)};
  int bexp = hd.u >> 52;
  hd.u &= ~0ULL>>12;
  if (bexp == 0x7ff) {    // nan or +/- inf
    M_apm_error(M_APM_RETURN,
      "M_set_double, Invalid input (likely a NAN or INF)");
    M_set_to_zero(r);
    return;
  }
  if (bexp) {             // normal number
    hd.u |= 1ULL<<52;     // add implied bits
    bexp -= BIAS + 54;
  } else {                // zero or subnormals
    if (hd.u == 0) {M_set_to_zero(r); return;}
    bexp -= BIAS + 53;
  }

  m_apm_set_uint64_t(r, hd.u);
  m_apm_ishift(r, bexp);
  if (d < 0) r->m_apm_sign = -1;
  m_apm_iround(r, places);
}
