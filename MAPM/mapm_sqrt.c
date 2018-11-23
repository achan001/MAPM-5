#include "m_apm.h"

void M_sqrt_flip(M_APM r, int places, M_APM N, int flip)
{
  if (N->m_apm_sign <= 0) {
    if (N->m_apm_sign || flip) 
      M_apm_error(M_APM_RETURN, "M_sqrt_flip, bad input");
    M_set_to_zero(r);
    return;
  }

  M_APM guess = M_get_stack_var();
  M_APM lastx = M_get_stack_var();
  M_APM tmp1  = M_get_stack_var();
  M_APM tmpN  = M_get_stack_var();

  /* normalize input number, exponent of tmpN = 1 or 2 */
  m_apm_copy(tmpN, N);
  int nexp = (N->m_apm_exponent - 1) >> 1;
  int nlen = N->m_apm_datalength;
  tmpN->m_apm_exponent -= nexp << 1;

  /* guess = 0.1 to 1 */
  double dd = m_apm_to_double(tmpN);
  m_apm_set_double(guess, 1e17 / sqrt(dd));
  guess->m_apm_exponent -= 17;

  int tolerance = - ((places + 6) >> 1);
  int maxp    = places + (places & 1) + 8;
  int dplaces = 28;     // BOTH maxp, dplaces MUST be EVEN

  /*  Use the following iteration to calculate 1 / sqrt(N) :

      X    = X + X (1 - N X^2) / 2 = X + 50 (X/100) (1 - N X^2)
       n+1
  */

  for (;;) {
    m_apm_square(tmp1, guess);
    m_apm_iround(tmp1, dplaces);
    tmpN->m_apm_datalength = nlen;
    m_apm_ichop(tmpN, dplaces + 1);     // non-destructive chop
    m_apm_multiply(r, tmpN, tmp1);      // N X^2
    m_apm_iround(r, dplaces);
    M_sub_samesign(tmp1, MM_One, r);    // 1 - N X^2

    int hplaces = dplaces + tmp1->m_apm_exponent;
    if (hplaces < 0) hplaces = 0;

    M_mul_digit(tmp1, tmp1, 50);        // 50 (1 - N X^2)
    m_apm_copy(lastx, guess);
    guess->m_apm_exponent -= 2;
    m_apm_iround(guess, hplaces + 1);   // X/100
    m_apm_multiply(r, tmp1, guess);
    m_apm_iround(r, hplaces);
    m_apm_add(guess, lastx, r);

    if (r->m_apm_exponent < tolerance)
      if (dplaces == maxp || places <= 22) break;

    if ((dplaces *= 2) > maxp) dplaces = maxp;
  }

  if (flip) {
    m_apm_copy(r, guess);
    r->m_apm_exponent -= nexp;          // r = 1 / sqrt(N)
  } else {
    m_apm_iround(guess, dplaces);
    m_apm_multiply(r, guess, tmpN);
    r->m_apm_exponent += nexp;          // r = sqrt(N)
  }
  m_apm_ichop(r, places + 6);           // avoid double rounding
  M_restore_stack(4);
}

void m_apm_sqrt(M_APM r, int places, M_APM N)
{
  M_sqrt_flip(r, places, N, FALSE);
  m_apm_iround(r, places);
}
