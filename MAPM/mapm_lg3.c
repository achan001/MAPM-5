#include "m_apm.h"

// r = 2*AGM(a,b), a,b also used for temp variables

static void M_2xAGM(M_APM r, int places, M_APM a, M_APM b)
{
    M_APM t = M_get_stack_var();
    places += 16;
    int tolerance = -(places/2U);   // assumed r ~ 1

    while (TRUE) {
        m_apm_add(r, a, b);
        m_apm_subtract(t, a, b);
        if (t->m_apm_exponent <= tolerance) break;
        m_apm_multiply(t, a, b);    // do AGM
        m_apm_sqrt(b, places, t);
        r->m_apm_exponent -= 2;
        M_mul_digit(a, r, 50);
        m_apm_iround(a, places);
    }
    M_restore_stack(1);
}

/****************************************************************************/

/*
 *                  Pi
 *  log10 = ----------------- (1 + O(1/10^(2k))
 *          2 AGM(k, 4k/10^k)
 *
 */

void M_check_log_places(int places)
{
    if ((places += 6) <= MM_lc_log_digits) return;

    MM_lc_log_digits = (places += 6);
    int k = places/2U + 6;

    M_APM a = M_get_stack_var();
    M_APM b = M_get_stack_var();
    M_APM r = M_get_stack_var();
    
    m_apm_set_unsigned(a, k);
    M_mul_digit(b, a, 4);
    b->m_apm_exponent -= k;
    M_2xAGM(r, places, a, b);   // r = 1.36437635384 ...

    M_check_PI_places(places);
    m_apm_divide(MM_lc_LOG_10, places, MM_lc_PI, r);
    m_apm_reciprocal(MM_lc_LOG_10R, places, MM_lc_LOG_10);
    M_restore_stack(3);
}
