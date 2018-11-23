#include "m_apm.h"

/*
 *    input  floor   ceil  integer  fraction
 *  -----------------------------------------
 *    100.5    100    101   100      0.5
 *    100.0    100    100   100      0.0
 *   -100.0   -100   -100  -100      0.0
 *   -100.5   -101   -100  -100     -0.5
 */

/****************************************************************************/

// r += sgn(r) * d, d = [1, 100]
// NOTE: if d = 0, r is untouched

void M_add_digit(M_APM r, int d)
{
  int n = r->m_apm_exponent;
  int even = !(n&1);
  if (d == 0) return;
  if (!even) d *= 10u;              // line-up M digit
  if (n <= 2) {                     // abs(r) < 100
    if (n > 0) d += r->m_apm_data[0];
    if (!even) d /= 10u;
    r->m_apm_sign |= 1;             // ASSUME 0 == +0
    M_set_samesign(r, d);
    return;
  }
  if (r->m_apm_datalength < n - even) M_apm_pad(r, 0, n);
  UCHAR *last = &r->m_apm_data[(n-1)>>1];
  r->m_apm_datalength = n;          // all digits of r

  int k = M_bcd_100[d += 100*last[-1] + last[0]];
  last[0] = k;
  last[-1] = k>>8;

  if (d >= 10000) {                 // fix overflow
    d -= 10000;
    if (!even) d /= 10u;            // last 2 digits
    else *last = 99;                // force round-up
    m_apm_iround(r, n-3);           // round up hundreds
    r->m_apm_datalength = n = r->m_apm_exponent;
    last = &r->m_apm_data[(n-1)>>1];
    last[0] = d;                    // add back 2 digits
    if (n & 1) {
      last[0] = d = M_bcd_100[10 * d];
      last[-1] += d >> 8;
    }
  }
  M_apm_normalize(r);
}

void m_apm_away(M_APM r, M_APM x)
{
  m_apm_copy(r, x);
  if (! m_apm_is_integer(x)) M_add_digit(r, 1);
}

void m_apm_integer(M_APM r, M_APM x)
{
  if (x->m_apm_exponent <= 0) {M_set_to_zero(r); return;}

  m_apm_copy(r, x);
  if (r->m_apm_exponent >= r->m_apm_datalength) return;

  r->m_apm_datalength = r->m_apm_exponent;
  M_apm_normalize(r);
}

void m_apm_fraction(M_APM r, M_APM x)
{
  if (m_apm_is_integer(x)) {M_set_to_zero(r); return;}
  m_apm_copy(r, x);
  int xexp = x->m_apm_exponent;
  if (xexp <= 0) return;  // x is a fraction

  int n = xexp >> 1;      // number of 00's
  if (xexp & 1) r->m_apm_data[n] %= 10U;
  memset(r->m_apm_data, 0, n);
  M_apm_normalize(r);
}

void m_apm_floor(M_APM r, M_APM x)
{
  if (x->m_apm_sign == -1)
    m_apm_away(r, x);
  else
    m_apm_integer(r, x);
}

void m_apm_ceil(M_APM r, M_APM x)
{
  if (x->m_apm_sign == +1)
    m_apm_away(r, x);
  else
    m_apm_integer(r, x);
}
