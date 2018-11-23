#include "m_apm.h"

// r = x ** n, all rounding in dplaces
// NOTE: r LESS accurate than dplaces
// NOTE: r last rounding performed elsewhere

void M_integer_pos_pow(M_APM r, int dplaces, M_APM x, unsigned n)
{
    if (n < 2) {m_apm_copy(r, n ? x : MM_One); return;}

    M_APM t1 = M_get_stack_var();
    M_integer_pos_pow(t1, dplaces, x, n>>1);
    m_apm_iround(t1, dplaces);
    if (n & 1) {
        M_APM t2 = M_get_stack_var();
        m_apm_multiply(t2, x, t1);
        m_apm_iround(t2, dplaces);
        m_apm_multiply(r, t1, t2);
        M_restore_stack(2);
        return;
    }
    m_apm_square(r, t1);
    M_restore_stack(1);
}

/****************************************************************************/

// r = x ** n, round to places

void m_apm_integer_pow(M_APM r, int places, M_APM x, int n)
{
    if (x->m_apm_sign == 0) {M_set_to_zero(r); return;}

    if (n >= 0) {
      M_integer_pos_pow(r, places + 8, x, n);
      m_apm_iround(r, places);
      return;
    }
    M_APM tmp = M_get_stack_var();
    M_integer_pos_pow(tmp, places + 8, x, -n);
    m_apm_reciprocal(r, places, tmp);
    M_restore_stack(1);
}

/****************************************************************************/

void m_apm_pow(M_APM r, int places, M_APM x, M_APM y)
{
    if (y->m_apm_sign == 0) {m_apm_copy(r, MM_One); return;}
    if (x->m_apm_sign == 0) {M_set_to_zero(r); return;}

    /*
     *  for small integer exponents,
     *  call the more efficient _integer_pow function.
     */

    if (INRANGE(y->m_apm_exponent, 1, 9))
        if (y->m_apm_exponent >= y->m_apm_datalength)
    {
        int n = lrint(m_apm_to_double(y));
        m_apm_integer_pow(r, places, x, n);
        return;
    }

    /* x ^ y = e ^ (y * log(x)) */

    M_APM tmp1 = M_get_stack_var();
    M_apm_log(r, places + 4, x);
    m_apm_iround(r, places + 8);
    m_apm_multiply(tmp1, y, r);
    m_apm_exp(r, places, tmp1);
    M_restore_stack(1);
}
