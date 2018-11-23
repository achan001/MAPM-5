#include "m_apm.h"

void m_apm_multiply(M_APM r, M_APM x, M_APM y)
{
  r->m_apm_sign = x->m_apm_sign * y->m_apm_sign;
  if (!r->m_apm_sign) {M_set_to_zero(r); return;}

  /* limit number of inner loops */
  if (x->m_apm_datalength > y->m_apm_datalength) SWAP(M_APM, x, y);
#if FAST_MUL >= 4
  if (x->m_apm_datalength > FAST_MUL) {M_fast_multiply(r, x, y); return;}
#endif
  int xbytes = (x->m_apm_datalength + 1) >> 1;
  int ybytes = (y->m_apm_datalength + 1) >> 1;
  int n = xbytes + ybytes;

  r->m_apm_exponent = x->m_apm_exponent + y->m_apm_exponent;
  r->m_apm_datalength = n * 2;

  if (n > r->m_apm_malloclength) M_realloc(r, n + 28);

  UCHAR *xp = x->m_apm_data - 1;
  UCHAR *yp = y->m_apm_data - 1;
  UCHAR *rp = r->m_apm_data + xbytes - 1;
  memset(r->m_apm_data, 0, n);

  do {
    int mul = xp[xbytes];
    if (mul) {
      int bcd = 0;
      for(int i=ybytes; i>0;) {
        bcd += mul * yp[i] + rp[i];
        rp[i--] = bcd = M_bcd_100[bcd];
        bcd >>= 8;
      }
      rp[0] = bcd;
    }
    rp--;
  } while (--xbytes);

  M_apm_normalize(r);
}

/****************************************************************************/
#ifndef NO_MAPM_SQR
void m_apm_square(M_APM r, M_APM x)
{
  if (x->m_apm_sign == 0) {M_set_to_zero(r); return;}
#if FAST_MUL >= 4
  if (x->m_apm_datalength > FAST_MUL) {M_fast_multiply(r, x, x); return;}
#endif
  int xbytes = (x->m_apm_datalength + 1) >> 1;
  int n = xbytes * 2;

  r->m_apm_sign = 1;
  r->m_apm_exponent = x->m_apm_exponent * 2;
  r->m_apm_datalength = n * 2;

  if (n > r->m_apm_malloclength) M_realloc(r, n + 28);

  UCHAR *xp = x->m_apm_data;
  UCHAR *rp = r->m_apm_data + xbytes;
  memset(rp, 0, xbytes);

  while(--xbytes) {
    int mul = xp[xbytes];
    int bcd = mul * mul + rp[xbytes];
    mul *= 2;     // doubled due to symmetry
    rp[xbytes] = bcd = M_bcd_100[bcd];
    bcd >>= 8;
    for(int i=xbytes; i--; ) {
      bcd += mul * xp[i] + rp[i];
      rp[i] = bcd = M_bcd_100[bcd];
      bcd >>= 8;
    }
    *--rp = bcd;  // might be upto 199
  }

  int bcd = xp[0] * xp[0] + rp[0];
  rp[0] = bcd = M_bcd_100[bcd];
  rp[-1] = bcd >> 8;
  M_apm_normalize(r);
}
#endif
/****************************************************************************/

// r = x * digit, 0 <= digit <= 99 (r can be same as x)
// For example, it is ok to do M_mul_digit(x, x, 99)

void M_mul_digit(M_APM r, M_APM x, int digit)
{
  if (!digit || !x->m_apm_sign) {M_set_to_zero(r); return;}

  int n = (x->m_apm_datalength + 1) >> 1;
  r->m_apm_sign = x->m_apm_sign;
  r->m_apm_exponent = x->m_apm_exponent + 2;
  r->m_apm_datalength = 2*n + 2;

  if (n >= r->m_apm_malloclength) M_realloc(r, n + 28);
  UCHAR *xp = x->m_apm_data - 1;
  UCHAR *rp = r->m_apm_data;
  int bcd = 0;

  do {
    bcd += digit * xp[n];
    rp[n] = bcd = M_bcd_100[bcd];
    bcd >>= 8;
  } while (--n);

  *rp = bcd;
  M_apm_normalize(r);
}
