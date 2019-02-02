#include "m_apm.h"

/****************************************************************************/
/*
 *      arcsinh(|x|) == log [ |x| + sqrt(x^2 + 1) ]
 *      arcsinh(-x) == - arcsinh(x)
 */
void m_apm_arcsinh(M_APM r, int places, M_APM x)
{
    M_APM tmp1 = M_get_stack_var();
    M_APM tmp2 = M_get_stack_var();

    m_apm_square(tmp1, x);
    M_add_samesign(tmp2, tmp1, MM_One);
    
    int more = -x->m_apm_exponent ; // x=0 -> INT32_MIN
    M_sqrt_flip(tmp1, places + (more>0 ? more : 0), tmp2, FALSE);
    
    M_add_samesign(tmp2, tmp1, x);  // keep sig. digits
    m_apm_log(r, places, tmp2);
    r->m_apm_sign = x->m_apm_sign;  // fix sign
    M_restore_stack(2);
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

    M_APM tmp1 = M_get_stack_var();
    M_APM tmp2 = M_get_stack_var();

    m_apm_square(tmp1, x);
    m_apm_subtract(tmp2, tmp1, MM_One);
    M_sqrt_flip(tmp1, places, tmp2, FALSE);
    m_apm_add(tmp2, tmp1, x);
    m_apm_log(r, places, tmp2); 
    M_restore_stack(2);
}
/****************************************************************************/
/*
 *      arctanh(x) == 0.5 * log [ (1 + x) / (1 - x) ]
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

    M_APM tmp1 = M_get_stack_var();
    M_APM tmp2 = M_get_stack_var();

    m_apm_add(tmp1, MM_One, x);
    m_apm_subtract(r, MM_One, x);
    
    // minimum exponent of x/10, x-1, x+1
    int mexp = x->m_apm_exponent - 1U;  // x = 0 -> INT32_MAX
    if (tmp1->m_apm_exponent < mexp) mexp = tmp1->m_apm_exponent;
    if (   r->m_apm_exponent < mexp) mexp =    r->m_apm_exponent;
    
    m_apm_divide(tmp2, places + 8 - mexp, tmp1, r);
    M_apm_log(r, places, tmp2);
    r->m_apm_exponent -= 2;
    M_mul_digit(r, r, 50);
    m_apm_iround(r, places);
    M_restore_stack(2);
}
/****************************************************************************/
