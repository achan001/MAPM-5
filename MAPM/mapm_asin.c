#include "m_apm.h"

void m_apm_arctan2(M_APM r, int places, M_APM y, M_APM x)
{
  int ix = x->m_apm_sign;
  int iy = y->m_apm_sign;

  if (ix == 0) {
    if (iy == 0) {
      M_apm_error(M_APM_RETURN, "m_apm_arctan2, Both Inputs = 0");
      M_set_to_zero(r);
      return;
    }
    M_check_PI_places(places);
    m_apm_round(r, places, MM_lc_HALF_PI);
    r->m_apm_sign = iy;
    return;
  }

  if (iy == 0) {
    if (ix == 1) {M_set_to_zero(r); return;}
    M_check_PI_places(places);
    m_apm_round(r, places, MM_lc_PI);
    return;
  }

  M_APM tmp = M_get_stack_var();

  if (ix == 1) {
    m_apm_divide(tmp, (places + 6), y, x);
    m_apm_arctan(r, places, tmp);
    M_restore_stack(1);
    return;
  }

  /* special cases have been handled, now do the real work */
  /* r = sign(-tmp) * (PI - abs(tmp)) */

  m_apm_divide(r, (places + 6), y, x);
  m_apm_arctan(tmp, (places + 6), r);
  M_check_PI_places(places);
  M_sub_samesign(r, tmp, MM_lc_PI);
  m_apm_iround(r, places);
  M_restore_stack(1);
}
/****************************************************************************/
/*
        Calculate arctan using the identity :

                                      x
        arctan (x) == arcsin [ --------------- ]
                                sqrt(1 + x^2)

*/
void m_apm_arctan(M_APM r, int places, M_APM x)
{
  if (x->m_apm_exponent < -1) {M_arctan_near_0(r, places, x); return;}
  if (x->m_apm_exponent >  3) {M_arctan_large_input(r, places, x); return;}

  M_APM tmp = M_get_stack_var();
  m_apm_square(r, x);
  m_apm_add(tmp, r, MM_One);
  M_sqrt_flip(r, places + 2, tmp, TRUE);
  m_apm_multiply(tmp, x, r);
  m_apm_iround(tmp, places + 6);
  m_apm_arcsin(r, places, tmp);
  M_restore_stack(1);
}
/****************************************************************************/
/*

    for large input values use :

    arctan(x) =  (PI / 2) - arctan(1 / |x|)

    and sign of result = sign of original input

*/

void M_arctan_large_input(M_APM r, int places, M_APM x)
{
  M_APM tmp = M_get_stack_var();
  M_reciprocal(r, places, x);
  M_arctan_near_0(tmp, places, r);

  M_check_PI_places(places);
  M_sub_samesign(r, MM_lc_HALF_PI, tmp);
  m_apm_iround(r, places);
  r->m_apm_sign = x->m_apm_sign;
  M_restore_stack(1);
}
/****************************************************************************/

void m_apm_arcsin(M_APM r, int places, M_APM x)
{
  switch (m_apm_compare_absolute(MM_One, x))
  {
    case 1: break;
    case 0: /* |x| == 1, arcsin = +/- PI / 2 */
      M_check_PI_places(places);
      m_apm_copy(r, MM_lc_HALF_PI);
      r->m_apm_sign = x->m_apm_sign;
      m_apm_iround(r, places);
      return;
    case -1:
      M_apm_error(M_APM_RETURN, "m_apm_arcsin, |Argument| > 1");
      M_set_to_zero(r);
      return;
  }

  if (m_apm_compare_absolute(x, MM_0_99995) >= 0) {
    M_APM tmp1 = M_get_stack_var();
    M_sin_to_cos(tmp1, places, x, FALSE);
    M_arccos_near_0(r, places, tmp1);
    r->m_apm_sign = x->m_apm_sign;
    M_restore_stack(1);
    return;
  }

  if (x->m_apm_exponent < -1) {M_arcsin_near_0(r, places, x); return;}

  // Done with special cases ...

  M_APM guess = M_get_stack_var();
  M_APM tmp1  = M_get_stack_var();
  M_APM tmp2  = M_get_stack_var();

  double dd = m_apm_to_double(x);
  m_apm_set_double(guess, 1e17 * asin(dd));
  if (guess->m_apm_sign) guess->m_apm_exponent -= 17;

  int tolerance = guess->m_apm_exponent - ((places + 6) >> 1);
  int maxp      = places + 8;
  int dplaces   = 28;

/*    Use the following iteration to solve for arc-sin :

                      sin(X) - N
      X     =  X  -  ------------
       n+1              cos(X)
*/

  for(;;)
  {
    int hplaces = (dplaces >> 1);
    M_625x_sin(tmp1, dplaces - 5, guess);
    m_apm_subtract(r, tmp1, x);
    m_apm_iround(r, hplaces + 1);

    M_sin_to_cos(tmp2, hplaces - 5, tmp1, TRUE);
    m_apm_multiply(tmp1, r, tmp2);
    m_apm_iround(tmp1, hplaces);
    m_apm_subtract(tmp2, guess, tmp1);

    if (tmp1->m_apm_exponent < tolerance)
      if (dplaces == maxp || places <= 22) break;

    if ((dplaces *= 2) > maxp) dplaces = maxp;
    SWAP(M_APM, guess, tmp2);
  }
  m_apm_round(r, places, tmp2);
  M_restore_stack(3);
}
/****************************************************************************/

void m_apm_arccos(M_APM r, int places, M_APM x)
{
  switch (m_apm_compare_absolute(MM_One, x))
  {
    case 1: break;
    case 0: /* |x| == 1, arccos = 0, PI */
      if (x->m_apm_sign == 1) {M_set_to_zero(r); return;}
      M_check_PI_places(places);
      m_apm_round(r, places, MM_lc_PI);
      return;
    case -1:
      M_apm_error(M_APM_RETURN, "m_apm_arccos, |Argument| > 1");
      M_set_to_zero(r);
      return;
  }

  if (m_apm_compare_absolute(x, MM_0_99995) >= 0) {
    M_APM tmp1 = M_get_stack_var();
    M_sin_to_cos(tmp1, places, x, FALSE);
    if (x->m_apm_sign == 1) {
      M_arcsin_near_0(r, places, tmp1);
      M_restore_stack(1);
      return;
    }
    M_APM tmp2 = M_get_stack_var();
    M_check_PI_places(places);
    M_arcsin_near_0(r, places + 4, tmp1);
    m_apm_subtract(r, MM_lc_PI, tmp2);
    m_apm_iround(r, places);
    M_restore_stack(2);
    return;
  }

  if (x->m_apm_exponent < -1) {M_arccos_near_0(r, places, x); return;}

  // Done with special cases ...

  M_APM guess = M_get_stack_var();
  M_APM tmp1  = M_get_stack_var();
  M_APM tmp2  = M_get_stack_var();

  double dd = m_apm_to_double(x);
  m_apm_set_double(guess, 1e17 * acos(dd));
  if (guess->m_apm_sign) guess->m_apm_exponent -= 17;

  int tolerance = guess->m_apm_exponent - ((places + 6) >> 1);
  int maxp      = places + 8;
  int dplaces   = 28;

/*    Use the following iteration to solve for arc-cos :

                      cos(X) - N
      X     =  X  +  ------------
       n+1              sin(X)
*/

  for(;;)
  {
    int hplaces = (dplaces >> 1);
    M_625x_cos(tmp1, dplaces - 5, guess);
    m_apm_subtract(r, tmp1, x);
    m_apm_iround(r, hplaces + 1);

    M_sin_to_cos(tmp2, hplaces - 5, tmp1, TRUE);
    m_apm_multiply(tmp1, r, tmp2);
    m_apm_iround(tmp1, hplaces);
    m_apm_add(tmp2, guess, tmp1);

    if (tmp1->m_apm_exponent < tolerance)
      if (dplaces == maxp || places <= 22) break;

    if ((dplaces *= 2) > maxp) dplaces = maxp;
    SWAP(M_APM, guess, tmp2);
  }
  m_apm_round(r, places, tmp2);
  M_restore_stack(3);
}
