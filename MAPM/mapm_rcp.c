#include "m_apm.h"

void m_apm_divide(M_APM r, int places, M_APM x, M_APM y)
{
#if FAST_DIV
    if (places < FAST_DIV) {
        M_apm_divide(r, places, x, y);
        m_apm_iround(r, places);
        return;
    }
    if ((x->m_apm_sign & y->m_apm_sign) == 0) {
        if (y->m_apm_sign == 0)
            M_apm_error(M_APM_RETURN, "m_apm_divide, Divide by 0");
        M_set_to_zero(r);
        return;
    }
    M_APM tmp = M_get_stack_var();
    M_reciprocal(tmp, places, y);
    m_apm_multiply(r, x, tmp);
    m_apm_iround(r, places);
    M_restore_stack(1);
#else   // slow, but guarantee correct rounding
    M_apm_divide(r, places, x, y);
    m_apm_iround(r, places);
#endif
}

/****************************************************************************/

void m_apm_reciprocal(M_APM r, int places, M_APM N)
{
    M_reciprocal(r, places, N);
    m_apm_iround(r, places);
}

void M_reciprocal(M_APM r, int places, M_APM N)
{
    if (N->m_apm_sign == 0) {
        M_apm_error(M_APM_RETURN, "M_reciprocal, Input = 0");
        M_set_to_zero(r);
        return;
    }

    M_APM tmp1  = M_get_stack_var();
    M_APM tmp2  = M_get_stack_var();
    M_APM tmpN  = M_get_stack_var();
    M_APM lastx = M_get_stack_var();

    /* normalize input number, exponent of tmpN = 1 */
    m_apm_copy(tmpN, N);
    int nlen = N->m_apm_datalength;
    tmpN->m_apm_exponent = 1;

    /* |guess| = 0.1 to 1 */
    double dd = m_apm_to_double(tmpN);
    m_apm_set_double(r, 1e17 / dd);
    r->m_apm_exponent -= 17;

    int tolerance = -((places + 6) >> 1);
    int maxp    = places + (places & 1) + 8;
    int dplaces = 28;       // maxp, dplaces MUST be EVEN

    /* Use the following iteration to calculate the reciprocal :

        X    = X + X (1 - N X)
         n+1
    */

    for (;;)
    {
        tmpN->m_apm_datalength = nlen;
        m_apm_ichop(tmpN, dplaces + 1);     // non-destructive chop
        m_apm_multiply(tmp1, tmpN, r);      // N X
        m_apm_iround(tmp1, dplaces);
        M_sub_samesign(tmp2, MM_One, tmp1); // 1 - N X

        int hplaces = dplaces + tmp2->m_apm_exponent;
        if (hplaces < 0) hplaces = 0;

        m_apm_copy(lastx, r);
        m_apm_iround(r, hplaces + 1);
        m_apm_multiply(tmp1, tmp2, r);      // X (1 - N X)
        m_apm_iround(tmp1, hplaces);
        m_apm_add(r, lastx, tmp1);

        if (tmp1->m_apm_exponent < tolerance)
          if (dplaces == maxp || places <= 22) break;

        if ((dplaces *= 2) > maxp) dplaces = maxp;
    }
    r->m_apm_exponent -= N->m_apm_exponent - 1;
    M_restore_stack(4);
}
