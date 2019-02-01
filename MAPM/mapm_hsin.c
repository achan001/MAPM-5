#include "m_apm.h"
#define min0(x)  ((x) & ((signed)(x)>>31))  // == min(0, x)

void M_exp_pair(M_APM t1, M_APM t2, int places, M_APM x)
{
  int sign = x->m_apm_sign;     // save sign
  x->m_apm_sign &= 1;           // x = |x|
  M_apm_exp(t1, places, x);
  x->m_apm_sign = sign;         // restore sign
  m_apm_iround(t1, places + 7); // t1 = exp(|x|)
  m_apm_square(t2, t1);         // t2 = t1 * t1
  m_apm_iround(t2, places + 7);
}

// LET t1 = exp(|x|), t2 = t1 * t1
// cosh(x) = (exp(x) + exp(-x)) / 2 = (t2 + 1) / (2 * t1)

void m_apm_cosh(M_APM r, int places, M_APM x)
{
    M_APM t1 = M_get_stack_var();
    M_APM t2 = M_get_stack_var();

    M_exp_pair(t1, r, places, x);
    M_add_samesign(t2, r, MM_One);
    m_apm_iround(t2, places + 6);
    M_mul_digit(t1, t1, 2);
    m_apm_divide(r, places, t2, t1);
    M_restore_stack(2);
}

// LET t1 = exp(|x|), t2 = t1 * t1
// sinh(x) = (exp(x) - exp(-x)) / 2 = (t2 - 1) / (2 * t1)
// sign of sinh(x) = sign of x

void m_apm_sinh(M_APM r, int places, M_APM x)
{
    M_APM t1 = M_get_stack_var();
    M_APM t2 = M_get_stack_var();

    M_exp_pair(t1, r, places - min0(x->m_apm_exponent - 1U), x);
    M_sub_samesign(t2, r, MM_One);
    m_apm_iround(t2, places + 6);
    m_apm_iround(t1, places + 6);
    M_mul_digit(t1, t1, 2);
    m_apm_divide(r, places, t2, t1);
    if (r->m_apm_sign) r->m_apm_sign = x->m_apm_sign;
    M_restore_stack(2);
}

// LET t1 = exp(|x|), t2 = t1 * t1
// tanh(x) = sinh(x) / cosh(x) = (t2 - 1) / (t2 + 1)
// sign of tanh(x) = sign of x

void m_apm_tanh(M_APM r, int places, M_APM x)
{
    M_APM t1 = M_get_stack_var();
    M_APM t2 = M_get_stack_var();

    M_exp_pair(t1, r, places - min0(x->m_apm_exponent - 1U), x);
    M_sub_samesign(t2, r, MM_One);
    M_add_samesign(t1, r, MM_One);
    m_apm_iround(t2, places + 6);
    m_apm_iround(t1, places + 6);
    m_apm_divide(r, places, t2, t1);
    if (r->m_apm_sign) r->m_apm_sign = x->m_apm_sign;
    M_restore_stack(2);
}
