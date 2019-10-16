#include "m_apm.h"

/* sin(x) = x - x^3/3! + x^5/5! - x^7/7! + ... */

void M_raw_sin(M_APM rr, int places, M_APM xx)
{
  int tolerance, dplaces, local_precision, m1;

  M_APM sum  = M_get_stack_var();
  M_APM term = M_get_stack_var();
  M_APM tmp7 = M_get_stack_var();
  M_APM tmp8 = M_get_stack_var();

  m_apm_square(rr, xx);

  if (places < 0) places = ~places; // do sinh(x)
  else    rr->m_apm_sign *= -1;     // do  sin(x)

  m_apm_copy(sum, xx);
  m_apm_copy(term, xx);

  dplaces   = (places + 8) - xx->m_apm_exponent;
  tolerance = 4 - dplaces - rr->m_apm_exponent;

  for(m1=2; term->m_apm_exponent >= tolerance; m1 += 2)
  {
    local_precision = dplaces + term->m_apm_exponent;
    m_apm_iround(rr, local_precision);
    m_apm_iround(term, local_precision);
    m_apm_multiply(tmp8, term, rr);
    m_apm_iround(tmp8, local_precision);
    
    m_apm_set_uint64_t(tmp7, m1 * (uint64_t)(m1 + 1));
    m_apm_divide(term, local_precision, tmp8, tmp7);
    m_apm_add(tmp7, sum, term);
    SWAP(M_APM, sum, tmp7);
  }
  m_apm_round(rr, places + 6, sum);
  M_restore_stack(4);
}

/****************************************************************************/

/* cos(x) = 1 - x^2/2! + x^4/4! - x^6/6! + ... */

static void M_raw_cos(M_APM rr, int places, M_APM xx)
{
  int tolerance, dplaces, local_precision, m1;

  M_APM sum  = M_get_stack_var();
  M_APM term = M_get_stack_var();
  M_APM tmp7 = M_get_stack_var();
  M_APM tmp8 = M_get_stack_var();

  m_apm_square(rr, xx);
  m_apm_iround(rr, dplaces = places + 8);
  rr->m_apm_sign *= -1;

  M_mul_digit_exp(term, rr, 50, -2);
  m_apm_add(sum, MM_One, term);

  tolerance = 4 - dplaces - term->m_apm_exponent;

  for(m1=3; term->m_apm_exponent >= tolerance; m1 += 2)
  {
    local_precision = dplaces + term->m_apm_exponent;
    m_apm_iround(rr, local_precision);
    m_apm_iround(term, local_precision);
    m_apm_multiply(tmp8, term, rr);
    m_apm_iround(tmp8, local_precision);

    m_apm_set_uint64_t(tmp7, m1 * (uint64_t)(m1 + 1));
    m_apm_divide(term, local_precision, tmp8, tmp7);
    m_apm_add(tmp7, sum, term);
    SWAP(M_APM, sum, tmp7);
  }
  m_apm_round(rr, places + 6, sum);
  M_restore_stack(4);
}
/****************************************************************************/
/*
 *     calculate the multiple angle identity for sin (5x), cos (5x)
 *
 *     sin (5x) == sin(x) [ 16 * sin^4 (x)  -  20 * sin^2 (x)  +  5 ]
 *     cos (5x) == cos(x) [ 16 * cos^4 (x)  -  20 * cos^2 (x)  +  5 ]
 */
static void M_5x_do_it(M_APM r, int dplaces, M_APM x)
{
    M_APM t2  = M_get_stack_var();
    M_APM t4  = M_get_stack_var();

    m_apm_square(t2, x);
    m_apm_iround(t2, dplaces);
    m_apm_square(t4, t2);
    m_apm_iround(t4, dplaces);

    M_mul_digit(t2, t2, 20);
    M_mul_digit(t4, t4, 16);
    M_add_samesign(r, t4, MM_Five);
    M_sub_samesign(t4, r, t2);
    m_apm_multiply(r, x, t4);
    m_apm_iround(r, dplaces);
    M_restore_stack(2);
}

/****************************************************************************/
void M_limit_angle_to_pi(M_APM r, int places, M_APM a)
{
    M_APM tmp = M_get_stack_var();
    M_check_PI_places(places + a->m_apm_exponent);

    m_apm_integer_div_rem(tmp, r, a, MM_lc_2_PI);

    if (m_apm_compare_absolute(r, MM_lc_PI) == 1) {
      m_apm_copy(tmp, r);
      M_sub_samesign(r, tmp, MM_lc_2_PI);
    }
    m_apm_iround(r, places);
    M_restore_stack(1);
}
/****************************************************************************/
void M_625x_sin(M_APM r, int places, M_APM x)
{
    M_APM t = M_get_stack_var();
    M_mul_digit_exp(t, x, 16, -4);  // t = x / 625
    M_raw_sin(r, places + 4, t);
    M_5x_do_it(t, places + 10, r);
    M_5x_do_it(r, places + 10, t);
    M_5x_do_it(t, places + 10, r);
    M_5x_do_it(r, places + 10, t);
    M_restore_stack(1);
}
/****************************************************************************/
void M_625x_cos(M_APM r, int places, M_APM x)
{
    M_APM t = M_get_stack_var();
    M_mul_digit_exp(t, x, 16, -4);  // t = x / 625
    M_raw_cos(r, places + 4, t);
    M_5x_do_it(t, places + 10, r);
    M_5x_do_it(r, places + 10, t);
    M_5x_do_it(t, places + 10, r);
    M_5x_do_it(r, places + 10, t);
    M_restore_stack(1);
}
/****************************************************************************/
void M_sin_cos(M_APM sinv, M_APM cosv, int places, M_APM aa)
{
    M_APM t = M_get_stack_var();
    M_limit_angle_to_pi(t, (places + 6), aa);
    M_625x_cos(cosv, places + 2, t);
    if (m_apm_compare_absolute(cosv, MM_0_99995) >= 0) {
      M_625x_sin(sinv, places, t);
    } else {
      M_sin_to_cos(sinv, places, cosv, FALSE);
      sinv->m_apm_sign = t->m_apm_sign;
    }
    m_apm_iround(sinv, places + 4);
    m_apm_iround(cosv, places + 4);
    M_restore_stack(1);
}
/****************************************************************************/
// compute  r = sqrt(1 - a ^ 2)    , i.e. sin -> cos
// if flip, r = 1 / sqrt(1 - a ^ 2), i.e. sin -> sec
// Return r chopped to places + 6

void M_sin_to_cos(M_APM r, int places, M_APM a, int flip)
{
    M_APM t = M_get_stack_var();
    m_apm_square(r, a);
    M_sub_samesign(t, MM_One, r);
    M_sqrt_flip(r, places, t, flip);
    M_restore_stack(1);
}

/****************************************************************************/
void m_apm_sin(M_APM r, int places, M_APM a)
{
    M_APM t = M_get_stack_var();
    M_limit_angle_to_pi(t, (places + 6), a);
    M_625x_sin(r, places, t);
    m_apm_iround(r, places);
    M_restore_stack(1);
}
/****************************************************************************/
void m_apm_cos(M_APM r, int places, M_APM a)
{
    M_APM t = M_get_stack_var();
    M_limit_angle_to_pi(t, (places + 6), a);
    M_625x_cos(r, places, t);
    m_apm_iround(r, places);
    M_restore_stack(1);
}
/****************************************************************************/
void m_apm_sin_cos(M_APM sinv, M_APM cosv, int places, M_APM aa)
{
  M_sin_cos(sinv, cosv, places, aa);
  m_apm_iround(sinv, places);
  m_apm_iround(cosv, places);
}
/****************************************************************************/
void m_apm_tan(M_APM r, int places, M_APM a)
{
    M_APM sinv = M_get_stack_var();
    M_APM cosv = M_get_stack_var();
    M_sin_cos(sinv, cosv, places, a);
    m_apm_divide(r, places, sinv, cosv);
    M_restore_stack(2);
}
/****************************************************************************/
