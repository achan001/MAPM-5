#include "m_apm.h"

void M_apm_log(M_APM r, int places, M_APM x)
{
  if (x->m_apm_sign != 1) {
    M_apm_error(M_APM_RETURN, "m_apm_log, bad input");
    M_set_to_zero(r);
    return;
  }

  int xexp = x->m_apm_exponent;
  M_APM tmp0 = M_get_stack_var();

  if ((unsigned) xexp < 2U) {       /* 0.01 < x < 10 */
    M_sub_samesign(tmp0, x, MM_One);
    if (tmp0->m_apm_exponent < -3)  /* |x-1| < 1e-3 */
      M_log_near_1(r, places, tmp0);
    else
      M_log_basic_iteration(r, places, x);
    M_restore_stack(1);
    return;
  }

  /* log (m * 10 ^ n) = log(m) + n * log(10) */
  
  int base = (xexp > 0);      /* log(m) and n same sign */
  x->m_apm_exponent = base;   /* 0.1 < m < 10 */
  M_log_basic_iteration(tmp0, places, x);
  x->m_apm_exponent = xexp;   /* restore x */

  M_APM tmp1 = M_get_stack_var();
  m_apm_set_long(r, xexp - base);
  M_check_log_places(places);
  m_apm_multiply(tmp1, r, MM_lc_LOG_10);
  
  m_apm_add(r, tmp0, tmp1);
  M_restore_stack(2);
}

/****************************************************************************/

void m_apm_log(M_APM r, int places, M_APM x)
{
  M_apm_log(r, places, x);
  m_apm_iround(r, places);
}

/****************************************************************************/

void m_apm_log10(M_APM r, int places, M_APM x)
{

  M_APM tmp = M_get_stack_var();
  M_apm_log(tmp, places, x);
  m_apm_iround(tmp, places + 6);

  // log10(x)  = log(x) / log(10)

  M_check_log_places(places);
  m_apm_multiply(r, tmp, MM_lc_LOG_10R);
  m_apm_iround(r, places);
  M_restore_stack(1);
}
