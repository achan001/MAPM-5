#include "m_apm.h"

/****************************************************************************/
/*
        Calculate arcsin using the identity :

                                      x
        arcsin (x) == arctan [ --------------- ]
                                sqrt(1 - x^2)

*/
void M_arcsin_near_0(M_APM rr, int places, M_APM aa)
{
  M_APM tmp5 = M_get_stack_var();
  M_APM tmp6 = M_get_stack_var();
  M_sin_to_cos(tmp5, places + 4, aa, TRUE);
  m_apm_multiply(tmp6, aa, tmp5);
  m_apm_iround(tmp6, places + 8);
  M_arctan_near_0(rr, places, tmp6);
  M_restore_stack(2);
}
/****************************************************************************/
/*
        Calculate arccos using the identity :

        arccos (x) == PI / 2 - arcsin (x)

*/
void M_arccos_near_0(M_APM rr, int places, M_APM aa)
{
  M_APM tmp = M_get_stack_var();

  M_check_PI_places(places);
  M_arcsin_near_0(tmp, (places + 4), aa);
  m_apm_subtract(rr, MM_lc_HALF_PI, tmp);
  m_apm_iround(rr, places);
  M_restore_stack(1);
}
/****************************************************************************/
/*
calculate arctan (x) with the following series:

                      x^3     x^5     x^7     x^9
arctan (x)  =   x  -  ---  +  ---  -  ---  +  ---  ...
                       3       5       7       9

*/

void M_arctan_near_0(M_APM rr, int places, M_APM aa)
{
  M_APM tmp0  = M_get_stack_var();
  M_APM tmpS  = M_get_stack_var();
  M_APM tmpR  = M_get_stack_var();
  M_APM term  = M_get_stack_var();
  M_APM digit = M_get_stack_var();

  m_apm_copy(term, aa);
  m_apm_copy(tmpS, aa);
  m_apm_square(rr, aa);
  rr->m_apm_sign *= -1;

  int local_precision = places + 8;
  int dplaces = local_precision - (unsigned) aa->m_apm_exponent;
  int tolerance = 4U - dplaces  - (unsigned) rr->m_apm_exponent;
  int m1 = 1;

  while (term->m_apm_exponent >= tolerance)
  {
    m_apm_iround(rr, local_precision);
    m_apm_multiply(tmp0, term, rr);

    local_precision = dplaces + tmp0->m_apm_exponent;
    if (local_precision < 0) break;
    SWAP(M_APM, tmp0, term);
    m_apm_iround(term, local_precision);

    m_apm_set_unsigned(digit, m1 += 2);
    m_apm_divide(tmp0, local_precision, term, digit);
    m_apm_add(tmpR, tmpS, tmp0);
    SWAP(M_APM, tmpS, tmpR);
  }
  m_apm_round(rr, places, tmpS);
  M_restore_stack(5);
}
/****************************************************************************/
