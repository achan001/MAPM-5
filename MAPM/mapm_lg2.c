#include "m_apm.h"

/*
 * Calculate log(N)
 * N = 0.1 to 10 -> log(N) = -2.3 to 2.3
 *
 * use M_log_35_places to get X = log(N) to 35 places
 *
 * let Y = N * exp(-X) - 1 (Y should be very small)
 *
 * log(N) = X + log(1 + Y) (use the efficient log_near_1 )
 */

void M_log_basic_iteration(M_APM r, int places, M_APM N)
{
  if (places <= 35) {M_log_35_places(r, N); return;}

  M_APM tmp0 = M_get_stack_var();
  M_APM tmp1 = M_get_stack_var();
  M_APM tmpX = M_get_stack_var();

  M_log_35_places(tmpX, N);
  m_apm_iround(tmpX, 35);

  tmpX->m_apm_sign *= -1;
  M_apm_exp(tmp1, places + 2, tmpX);
  m_apm_iround(tmp1, places + 8);
  tmpX->m_apm_sign *= -1;

  m_apm_multiply(tmp0, tmp1, N);
  m_apm_subtract(tmp1, tmp0, MM_One);
  m_apm_iround(tmp1, places - 29);
  M_log_near_1(tmp0, (places - 35), tmp1);
  m_apm_add(r, tmpX, tmp0);
  M_restore_stack(3);
}

/****************************************************************************/

/*
 * calculate log(N) 35 places accurate
 * N = 0.1 to 10 -> log(N) = -2.3 to 2.3
 *
 * Use the following iteration to estimate for log :
 * This is a cubically convergent algorithm
 *
 *               exp(X) - N
 * r =  X - 2 * ------------ , X = estimate of log(N)
 *               exp(X) + N
 *
 * Accuracy of X = 13 to 14 places
 * Accuracy of r = 40 places > 35 places
*/

void M_log_35_places(M_APM r, M_APM N)
{
  M_APM tmp0 = M_get_stack_var();
  M_APM tmp1 = M_get_stack_var();
  M_APM tmpX = M_get_stack_var();

  double dd = m_apm_to_double(N);
  m_apm_set_double(tmpX, 1e17 * log(dd));
  if (tmpX->m_apm_sign) tmpX->m_apm_exponent -= 17;
  M_apm_exp(r, 35, tmpX);
  m_apm_iround(r, 41);

  M_sub_samesign(tmp0, r, N);
  M_add_samesign(tmp1, r, N);
  m_apm_iround(tmp0, 41);
  m_apm_iround(tmp1, 41);

  M_apm_divide(r, 41 - 15, tmp0, tmp1);
  M_mul_digit(tmp0, r, 2);
  m_apm_subtract(r, tmpX, tmp0);
  M_restore_stack(3);
}

/****************************************************************************/

/*
 * calculate log (1 + x) with the following series:
 *         x
 *   y = -----      ( |y| < 1 )
 *       x + 2
 *
 *       [ 1 + y ]                 y^3     y^5     y^7
 *   log [-------]  =  2 * [ y  +  ---  +  ---  +  ---  ... ]
 *       [ 1 - y ]                  3       5       7
*/

void M_log_near_1(M_APM r, int places, M_APM xx)
{
  M_APM tmpS = M_get_stack_var();
  M_APM tmp0 = M_get_stack_var();
  M_APM tmp1 = M_get_stack_var();
  M_APM term = M_get_stack_var();

  places += 12U - xx->m_apm_exponent;
  int dplaces = places + 6U;

  m_apm_add(term, xx, MM_Two);
  m_apm_divide(tmpS, dplaces, xx, term);
  m_apm_copy(term, tmpS);
  m_apm_square(r, tmpS);

  int tolerance = 6U - places - r->m_apm_exponent;

  for(int m1=3; term->m_apm_exponent >= tolerance; m1+=2)
  {
    m_apm_iround(r, dplaces);
    m_apm_multiply(tmp0, term, r);

    dplaces = places + tmp0->m_apm_exponent;
    SWAP(M_APM, tmp0, term);
    m_apm_iround(term, dplaces);

    m_apm_set_unsigned(tmp1, m1);
    m_apm_divide(tmp0, dplaces, term, tmp1);
    m_apm_add(tmp1, tmpS, tmp0);
    SWAP(M_APM, tmpS, tmp1);
  }
  M_mul_digit(r, tmpS, 2);
  M_restore_stack(4);
}
