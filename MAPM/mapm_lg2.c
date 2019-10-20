#include "m_apm.h"

#ifdef __GNUC__
#define LOG1P(x)  log1p(x)
#else   // tcc does not have log1p()
#define LOG1P(x)  ({double _y=1+(x); log(_y)-(_y-1-(x))/_y;})
#endif

/*
 * calculate log(N), 0.1 <= N < 10 and |N-1| >= 1e-3
 *
 * Use the following iteration to estimate for log :
 * This is a cubically convergent algorithm
 *
 *               exp(X) - N
 * r =  X - 2 * ------------ , X = estimate of log(N)
 *               exp(X) + N
 *
 * Accuracy of X = 13 to 14 places
 * Accuracy of r ~ 40 places > 35 places
*/

static void M_log_35_places(M_APM r, M_APM N)
{
  M_APM tmp0 = M_get_stack_var();
  M_APM tmp1 = M_get_stack_var();
  M_APM tmpX = M_get_stack_var();

  double dd = m_apm_to_double(r);   // assumed r = N-1
  m_apm_set_double(tmpX, 1e19 * LOG1P(dd));
  tmpX->m_apm_exponent -= 19;       // |X| > 0.0009995
  M_apm_exp(r, 35, tmpX);
  m_apm_iround(r, 43);

  M_sub_samesign(tmp0, r, N);
  M_add_samesign(tmp1, r, N);
  m_apm_iround(tmp0, 43 - 14);
  m_apm_iround(tmp1, 43 - 14);

  M_apm_divide(r, 43 - 16, tmp0, tmp1);
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

static void M_log_near_1(M_APM r, int places, M_APM x)
{
  M_APM tmpS = M_get_stack_var();
  M_APM tmp0 = M_get_stack_var();
  M_APM tmp1 = M_get_stack_var();
  M_APM xsqr = M_get_stack_var();
  M_APM term = M_get_stack_var();

  places += 12U - x->m_apm_exponent;
  int dplaces = places + 6U;

  m_apm_add(term, x, MM_Two);
  m_apm_divide(tmpS, dplaces, x, term);
  m_apm_copy(term, tmpS);
  m_apm_square(xsqr, tmpS);

  int tolerance = 6U - places - xsqr->m_apm_exponent;

  for(int m1=3; term->m_apm_exponent >= tolerance; m1+=2)
  {
    m_apm_iround(xsqr, dplaces);
    m_apm_multiply(tmp0, term, xsqr);

    dplaces = places + tmp0->m_apm_exponent;
    SWAP(M_APM, tmp0, term);
    m_apm_iround(term, dplaces);

    m_apm_set_unsigned(tmp1, m1);
    m_apm_divide(tmp0, dplaces, term, tmp1);
    m_apm_add(tmp1, tmpS, tmp0);
    SWAP(M_APM, tmpS, tmp1);
  }
  M_mul_digit(r, tmpS, 2);  // NOTE: M_log_near_1(r, places, r) OK
  M_restore_stack(5);
}

/****************************************************************************/

/*
 * Calculate log(N), 0.1 <= N < 10
 *
 * use M_log_35_places to get X = log(N) to 35 places
 *
 * let Y = N * exp(-X) - 1 (Y should be very small)
 *
 * log(N) = X + log(1 + Y) (use the efficient log_near_1 )
 */

void M_log_basic_iteration(M_APM r, int places, M_APM N)
{
  m_apm_subtract(r, N, MM_One);
  if (r->m_apm_exponent < -3) {M_log_near_1(r, places, r); return;}

  M_log_35_places(r, N);    // NOTE: required r = N-1
  if (places <= 35) return;

  M_APM tmp0 = M_get_stack_var();
  M_APM tmpX = M_get_stack_var();

  m_apm_negate(tmpX, r);
  M_apm_exp(r, places + 2, tmpX);
  m_apm_iround(r, places + 8);

  m_apm_multiply(tmp0, r, N);
  m_apm_subtract(r, tmp0, MM_One);
  m_apm_iround(r, places - 30);

  M_log_near_1(tmp0, places - 36, r);
  m_apm_subtract(r, tmp0, tmpX);
  M_restore_stack(2);
}
