#include "m_apm.h"

//
// check if our local copy of PI is precise enough
// for our purpose. if not, calculate PI so it's
// as precise as desired, accurate to 'places' decimal
// places.
//
void M_check_PI_places(int places)
{
  if ((places += 6) <= MM_lc_PI_digits) return;

  MM_lc_PI_digits = (places += 6);
  M_calculate_PI_AGM(MM_lc_PI, places);

  M_mul_digit(MM_lc_2_PI, MM_lc_PI, 2);
  M_mul_digit(MM_lc_HALF_PI, MM_lc_PI, 50);
  MM_lc_HALF_PI->m_apm_exponent -= 2;
}

//
//  Calculate PI using the AGM (Arithmetic-Geometric Mean)
//
//  Init :  A0  = 1
//          B0  = 1 / sqrt(2)
//          Sum = 1
//
//  Iterate: n = 1...
//
//
//  A   =  0.5 * [ A    +  B   ]
//   n              n-1     n-1
//
//
//  B   =  sqrt [ A    *  B   ]
//   n             n-1     n-1
//
//
//  C   =  A    -  B
//   n      n-1     n-1
//
//
//                   2      n-1
//  Sum  =  Sum  -  C   *  2
//                   n
//
//
//  At the end when C  is 'small enough' :
//                   n
//
//                2
//  PI  =  4  *  A    /  Sum
//                n+1
//
//      -OR-
//
//                   2
//  PI  = ( A  +  B )   /  Sum
//           n     n
//

void M_calculate_PI_AGM(M_APM outv, int places)
{
  M_APM a0    = M_get_stack_var();
  M_APM b0    = M_get_stack_var();
  M_APM c0    = M_get_stack_var();
  M_APM tmp1  = M_get_stack_var();
  M_APM tmp2  = M_get_stack_var();
  M_APM sum   = M_get_stack_var();
  M_APM pow_2 = M_get_stack_var();

  int dplaces = places + 16;

  m_apm_copy(a0, MM_One);
  m_apm_copy(sum, MM_One);
  m_apm_copy(pow_2, MM_One);
  m_apm_sqrt(b0, dplaces, MM_0_5);    // sqrt(0.5)

  do {
    m_apm_subtract(c0, a0, b0);
    m_apm_multiply(tmp2, a0, b0);
    m_apm_add(tmp1, a0, b0);

    SWAP(M_APM, a0, tmp1);
    a0->m_apm_exponent -= 2;
    M_mul_digit(a0, a0, 50);

    m_apm_sqrt(b0, dplaces, tmp2);

    m_apm_square(tmp1, c0);
    m_apm_multiply(tmp2, tmp1, pow_2);
    M_mul_digit(pow_2, pow_2, 2);

    m_apm_subtract(tmp1, sum, tmp2);
    m_apm_iround(tmp1, dplaces);
    SWAP(M_APM, tmp1, sum);

  } while (-4 * c0->m_apm_exponent < dplaces);

  m_apm_add(tmp1, a0, b0);
  m_apm_square(tmp2, tmp1);
  m_apm_iround(tmp2, dplaces);
  m_apm_divide(outv, places, tmp2, sum);
  M_restore_stack(7);
}
/****************************************************************************/
