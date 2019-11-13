#include "m_apm.h"

// r = 2*AGM(a,b), assumed r ~ 1
// Note: a,b also used for temp variables

static void M_2xAGM(M_APM r, int places, M_APM a, M_APM b)
{
    M_APM s = M_get_stack_var();
    places += 16;
    int tolerance = -(places/4U);

    while (TRUE) {
        m_apm_add(s, a, b);
        m_apm_subtract(r, a, b);
        if (r->m_apm_exponent < tolerance) break;
        m_apm_multiply(r, a, b);    // do AGM
        m_apm_sqrt(b, places, r);
        s->m_apm_exponent -= 2;
        M_mul_digit(a, s, 50);
        m_apm_iround(a, places);
    }
    M_mul_digit(r, r, 50);          // 2*AGM = s - r^2/(4s)
    r->m_apm_exponent -= 2;
    m_apm_square(a, r);
    m_apm_divide(b, places + a->m_apm_exponent, a, s);
    m_apm_subtract(r, s, b);
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
