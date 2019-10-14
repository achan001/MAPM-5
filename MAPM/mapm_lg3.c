#include "m_apm.h"

/*
 *  using the 'R' function (defined below) for 'N' decimal places :
 *
 *
 *                          -N             -N
 *  log(2)  =  R(1, 0.5 * 10  )  -  R(1, 10  )
 *
 *
 *                          -N             -N
 *  log(10) =  R(1, 0.1 * 10  )  -  R(1, 10  )
 *
 *
 *  In general:
 *
 *                    -N                -N
 *  log(x)  =  R(1, 10  / x)  -  R(1, 10  )
 *
 *
 *  I found this on a web site which went into considerable detail
 *  on the history of log(2). This formula is algebraically identical
 *  to the formula specified in J. Borwein and P. Borwein's book
 *  "PI and the AGM". (reference algorithm 7.2)
 */

/****************************************************************************/
void M_check_log_places(int places)
{
    if ((places += 6) <= MM_lc_log_digits) return;

    MM_lc_log_digits = places + 6;
    places += M_ilog10(places) + 6;

    M_APM tmp7 = M_get_stack_var();
    M_APM tmp8 = M_get_stack_var();
    M_APM tmp9 = M_get_stack_var();

    m_apm_copy(tmp7, MM_One);
    tmp7->m_apm_exponent = -(places/2u + places/16u);

    M_log_AGM_R_func(tmp8, places, MM_One, tmp7);

    tmp7->m_apm_exponent -= 1;
    M_log_AGM_R_func(tmp9, places, MM_One, tmp7);

    m_apm_subtract(MM_lc_LOG_10, tmp9, tmp8);
    m_apm_reciprocal(MM_lc_LOG_10R, places, MM_lc_LOG_10);
    M_restore_stack(3);

}
/****************************************************************************/

/*
 * define a notation for a function 'R' :
 *
 *
 *
 *                                    1
 *      R (a0, b0)  =  ------------------------------
 *
 *                          ----
 *                           \
 *                            \     n-1      2    2
 *                      1  -   |   2    *  (a  - b )
 *                            /              n    n
 *                           /
 *                          ----
 *                         n >= 0
 *
 *
 *      where a, b are the classic AGM iteration :
 *
 *
 *      a    =  0.5 * (a  + b )
 *       n+1            n    n
 *
 *
 *      b    =  sqrt(a  * b )
 *       n+1          n    n
 *
 *
 *
 *      Define a variable 'c' for more efficient computation :
 *
 *                             2        2     2
 *      c = a   -  b    ==>   c  = 4 ( a  -  b )
 *           n-1    n-1                 n     n
 *
 */

/****************************************************************************/
void M_log_AGM_R_func(M_APM rr, int places, M_APM aa, M_APM bb)
{
    M_APM tmpA0 = M_get_stack_var();
    M_APM tmpB0 = M_get_stack_var();
    M_APM tmpC2 = M_get_stack_var();
    M_APM tmp1  = M_get_stack_var();
    M_APM tmp2  = M_get_stack_var();
    M_APM sum   = M_get_stack_var();

    int tolerance = -((places + 8) >> 1);
    int dplaces   = places + 16;

    m_apm_copy(tmpA0, aa);
    m_apm_copy(tmpB0, bb);
    m_apm_square(rr, MM_0_5);       /* rr = 0.25 */

    m_apm_square(tmp1, aa);         /* sum = 0.5*(a^2 - b^2) */
    m_apm_square(tmp2, bb);
    m_apm_subtract(sum, tmp1, tmp2);
    sum->m_apm_exponent -= 2;
    M_mul_digit(sum, sum, 50);
    
    while (TRUE) {
        m_apm_subtract(tmp1, tmpA0, tmpB0);     /* C = An - Bn */
        m_apm_square(tmpC2, tmp1);              /* C ^ 2 */        
        m_apm_multiply(tmp1, rr, tmpC2);
        m_apm_add(tmp2, sum, tmp1);
        
        if (tmp1->m_apm_exponent < tolerance) break;
        
        M_mul_digit(rr, rr, 2);
        SWAP(M_APM, sum, tmp2);
        m_apm_iround(sum, dplaces);

        m_apm_add(tmp1, tmpA0, tmpB0);          /* do the AGM */
        m_apm_multiply(tmp2, tmpA0, tmpB0);
        m_apm_sqrt(tmpB0, dplaces, tmp2);
        tmp1->m_apm_exponent -= 2;
        M_mul_digit(tmpA0, tmp1, 50);
        m_apm_iround(tmpA0, dplaces);
    }

    m_apm_subtract(tmp1, MM_One, tmp2);
    m_apm_reciprocal(rr, places, tmp1);

    M_restore_stack(6);
}
/****************************************************************************/
