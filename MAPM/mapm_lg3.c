#include "m_apm.h"

// r = 2*AGM(1,b)

static void M_2xAGM1(M_APM r, int places, M_APM b)
{
    M_APM a = M_get_stack_var();
    M_APM t = M_get_stack_var();

    m_apm_copy(a, MM_One);

    places += 16;
    int tolerance = -(places/2U);

    while (TRUE) {
        m_apm_add(r, a, b);
        m_apm_subtract(t, a, b);
        if (t->m_apm_exponent < tolerance) break;

        m_apm_multiply(t, a, b);        // do AGM
        m_apm_sqrt(b, places, t);
        r->m_apm_exponent -= 2;
        M_mul_digit(a, r, 50);
        m_apm_iround(a, places);
    }
    M_restore_stack(2);
}

/****************************************************************************/

/*
 *                Pi
 *  log10 = -----------------  + O(1/10^(2k))
 *          2k AGM(1, 4/10^k)
 *
 */

void M_check_log_places(int places)
{
    if ((places += 6) <= MM_lc_log_digits) return;

    MM_lc_log_digits = (places += 6);
    int k = places/2U + 6;

    M_APM t = M_get_stack_var();
    M_APM x = M_get_stack_var();
    M_APM y = M_get_stack_var();

    m_apm_copy(t, MM_Four);
    t->m_apm_exponent -= k;
    M_2xAGM1(x, places, t);
    m_apm_set_unsigned(t, k);
    m_apm_multiply(y, t, x);

    M_check_PI_places(places);
    m_apm_divide(MM_lc_LOG_10, places, MM_lc_PI, y);
    m_apm_reciprocal(MM_lc_LOG_10R, places, MM_lc_LOG_10);
    M_restore_stack(3);
}
