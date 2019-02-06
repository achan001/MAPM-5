#include "m_apm.h"

/****************************************************************************/
/*
 *      arcsinh(x) == log [ |x| + sqrt(x^2 + 1) ], with sign of x
 */
void m_apm_arcsinh(M_APM r, int places, M_APM x)
{
    M_APM tmp = M_get_stack_var();
    m_apm_square(r, x);
    M_add_samesign(tmp, r, MM_One);

    int nexp = x->m_apm_exponent - 1U;  // x=0 -> INT32_MAX
    if (nexp >= 0 || -nexp > places+3) nexp = 0;
    M_sqrt_flip(r, places - nexp, tmp, FALSE);

    M_add_samesign(tmp, r, x);
    m_apm_log(r, places, tmp);
    r->m_apm_sign = x->m_apm_sign;
    M_restore_stack(1);
}

/****************************************************************************/
/*
 *      arccosh(x) == log [ x + sqrt(x^2 - 1) ]
 *
 *      x >= 1.0
 */
void m_apm_arccosh(M_APM r, int places, M_APM x)
{
    if (m_apm_compare(x, MM_One) < 0) {
        M_apm_error(M_APM_RETURN, "m_apm_arccosh, Argument < 1");
        M_set_to_zero(r);
        return;
    }

    M_APM tmp = M_get_stack_var();
    m_apm_square(r, x);
    M_sub_samesign(tmp, r, MM_One);

    M_sqrt_flip(r, places, tmp, FALSE);
    M_add_samesign(tmp, r, x);
    m_apm_log(r, places, tmp);
    M_restore_stack(1);
}
/****************************************************************************/
/*
 *      arctan(x) = 0.5 * log(2/(1 - |x|) - 1), with sign of x
 *
 *      |x| < 1.0
 */

void m_apm_arctanh(M_APM r, int places, M_APM x)
{
    if (m_apm_compare_absolute(x, MM_One) >= 0) {
        M_apm_error(M_APM_RETURN, "m_apm_arctanh, |Argument| >= 1");
        M_set_to_zero(r);
        return;
    }

    M_APM tmp = M_get_stack_var();
    M_sub_samesign(tmp, MM_One, x);

    int nexp = x->m_apm_exponent - 1U;  // x=0 -> INT32_MAX
    if (tmp->m_apm_exponent <= nexp) nexp = tmp->m_apm_exponent - 1U;
    M_reciprocal(r, places - nexp, tmp);

    M_mul_digit(r, r, 2);
    M_sub_samesign(tmp, r, MM_One);
    M_apm_log(r, places, tmp);

    r->m_apm_sign = x->m_apm_sign;
    r->m_apm_exponent -= 2;
    M_mul_digit(r, r, 50);
    m_apm_iround(r, places);
    M_restore_stack(1);
}
/****************************************************************************/
