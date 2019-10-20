#include "m_apm.h"
#define max(a,b)  ((a)>(b)?(a):(b))

#ifdef __GNUC__
#define GUESS(x)  (1/cbrt(x))
#else   // tcc does not have cbrt()
#define GUESS(x)  ({double _s=1-2*(x<0); _s * pow(_s*(x), -1/3.);})
#endif

void m_apm_cbrt(M_APM r, int places, M_APM N)
{
    if (N->m_apm_sign == 0) {M_set_to_zero(r); return;}

    M_APM guess = M_get_stack_var();
    M_APM lastx = M_get_stack_var();
    M_APM tmp1  = M_get_stack_var();
    M_APM tmpN  = M_get_stack_var();

    /* normalize input number, exponent of tmpN = 1, 2, 3 */
    m_apm_copy(tmpN, N);
    int nexp = N->m_apm_exponent - 1;
    int nlen = N->m_apm_datalength;
    nexp += 2*(nexp>>31);
    tmpN->m_apm_exponent -= 3 * (nexp/=3);

    /* |guess| = 0.1 to 1 */
    double dd = m_apm_to_double(tmpN);
    m_apm_set_double(guess, 1e17 * GUESS(dd));
    guess->m_apm_exponent -= 17;

    int tolerance = guess->m_apm_exponent - ((places + 6) >> 1);
    int maxp    = places + (places & 1) + 8;
    int dplaces = 28;       // maxp, dplaces MUST be EVEN

    /*   Use the following iteration to calculate 1 / cbrt(N) :

        X    = X + (X - N X ^ 4) / 3
         n+1
    */

    M_APM third = M_get_stack_var();
    third->m_apm_sign = 1;
    third->m_apm_exponent = 0;
    third->m_apm_datalength = 0;
    M_apm_pad(third, 33, (max(dplaces, maxp) >> 1) + 2);

    for (;;)
    {
        m_apm_square(r, guess);           // X * X
        m_apm_iround(r, dplaces);
        m_apm_square(tmp1, r);            // X ^ 4
        m_apm_iround(tmp1, dplaces);
        tmpN->m_apm_datalength = nlen;
        m_apm_ichop(tmpN, dplaces + 1);   // non-destructive chop

        m_apm_multiply(r, tmpN, tmp1);    // N X ^ 4
        m_apm_iround(r, dplaces);
        M_sub_samesign(tmp1, guess, r);   // X - N * X ^ 4

        third->m_apm_datalength = (dplaces>>1) + 2;
        m_apm_multiply(r, tmp1, third);   // divide by 3
        m_apm_iround(r, dplaces>>1);
        m_apm_add(lastx, guess, r);

        if (r->m_apm_exponent < tolerance)
          if (dplaces == maxp || places <= 22) break;

        if ((dplaces *= 2) > maxp) dplaces = maxp;
        SWAP(M_APM, guess, lastx);
    }
    m_apm_iround(lastx, dplaces);
    m_apm_square(guess, lastx);
    m_apm_iround(guess, dplaces);
    m_apm_multiply(r, guess, tmpN);
    m_apm_iround(r, places);
    r->m_apm_exponent += nexp;
    M_restore_stack(5);
}
