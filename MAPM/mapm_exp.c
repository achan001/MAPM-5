#include "m_apm.h"

static void M_raw_exp(M_APM r, int places, M_APM x)
{
  int term_exp = x->m_apm_exponent;
  int tolerance = - (places += term_exp);
  if (places < 0) {m_apm_copy(r, MM_One); return;}

  M_APM term = M_get_stack_var();
  M_APM sum  = M_get_stack_var();
  M_APM t1   = M_get_stack_var();
  M_APM s1   = M_get_stack_var();

  m_apm_round(r, places, x);
  m_apm_copy(term, r);
  m_apm_copy(sum, r);

  // exp(x) = 1 + x + x^2/2! + ...
  // |x| < 1E-4 (from M_apm_exp)

  for (int n=2 ; term_exp >= tolerance ; n++) {
    m_apm_set_unsigned(t1, n);
    m_apm_divide(s1, places, r, t1);
    m_apm_multiply(t1, s1, term);
    m_apm_iround(t1, places);
    m_apm_add(s1, t1, sum);

    places -= term_exp - t1->m_apm_exponent;
    term_exp = t1->m_apm_exponent;
    m_apm_iround(r, places);
    m_apm_iround(t1, places);
    SWAP(M_APM, t1, term);
    SWAP(M_APM, s1, sum);
  }
  m_apm_add(r, sum, MM_One);
  M_restore_stack(4);
}

void M_apm_exp(M_APM r, int places, M_APM x)
{
  int n = 0;
  places += 14;
  M_APM y = M_get_stack_var();

  if (x->m_apm_exponent > 2) {    // |x| >= 100
    if (x->m_apm_exponent > 9) {
      M_apm_error(M_APM_RETURN, "M_apm_exp, bad input");
      M_set_to_zero(r);
      return;
    }
    n = lrint(m_apm_to_double(x) * M_LOG10E);
    m_apm_set_long(y, n);
    M_check_log_places(places);   // x = y + n log10
    m_apm_multiply(r, y, MM_lc_LOG_10);
    m_apm_subtract(y, x, r);      // |y| <= log10 / 2 = 1.15
    x = y;
  }

  m_apm_multiply(r, x, MM_1024R); // max(|x|) = 100
  m_apm_multiply(y, r, MM_1024R); // max(|y|) = 100/2^20 < 1E-4
  M_raw_exp(r, places, y);        // exp(y) ~= 1 + y

  for(int i=10 ; i--;) {          // exp(x) = exp(y) ^ (2^20)
    m_apm_square(y, r);
    m_apm_iround(y, places);
    m_apm_square(r, y);
    m_apm_iround(r, places);
  }
  r->m_apm_exponent += n;
  M_restore_stack(1);
}

void m_apm_exp(M_APM r, int places, M_APM x)
{
  M_apm_exp(r, places, x);
  m_apm_iround(r, places);
}
