#include "m_apm.h"

/*
 * From Knuth, The Art of Computer Programming:
 *
 * This is the binary GCD algorithm as described
 * in the book (Algorithm B)
 */

void m_apm_gcd(M_APM r, M_APM u, M_APM v)
{
  if (!m_apm_is_integer(u) || !m_apm_is_integer(v))
  {
     M_apm_error(M_APM_RETURN, "m_apm_gcd, Non-integer input");
     M_set_to_zero(r);
     return;
  }

  if (u->m_apm_sign == 0) {m_apm_absolute_value(r, v); return;}
  if (v->m_apm_sign == 0) {m_apm_absolute_value(r, u); return;}

  M_APM tmpU = M_get_stack_var();
  M_APM tmpV = M_get_stack_var();
  M_APM tmpT = M_get_stack_var();

  m_apm_copy(tmpU, u);
  m_apm_copy(tmpV, v);

  int kk = 0; // collect factors of 2
  for(v = tmpV ;; kk++)
  {
     if (m_apm_is_odd(tmpU)) goto B4;
     if (m_apm_is_odd(tmpV)) break;

     M_mul_digit(tmpU, tmpU, 50); tmpU->m_apm_exponent -= 2;
     M_mul_digit(tmpV, tmpV, 50); tmpV->m_apm_exponent -= 2;
  }

  tmpV = tmpU; tmpU = v; v = tmpV;

B3:           // v MUST be even

  M_mul_digit(v, v, 50); v->m_apm_exponent -= 2;

B4:           // v to odd loop

  if (m_apm_is_even(v)) goto B3;

  M_sub_samesign(v = tmpT, tmpU, tmpV);

  switch (tmpT->m_apm_sign ^ tmpU->m_apm_sign) {
    case 0:   // |u| > |v| => gcd(v, t/2)
      tmpT = tmpU; tmpU = tmpV; tmpV = v; goto B3;
    case -2:  // |u| < |v| => gcd(u, t/2)
      tmpT = tmpV; tmpV = v; goto B3;
  }

  m_apm_absolute_value(r, tmpU);
  m_apm_ishift(r, kk);  // add factor 2 ^ kk
  M_restore_stack(3);
}

/****************************************************************************/

/*
 *                      u * v
 *      LCM(u,v)  =  ------------
 *                     GCD(u,v)
 */

void m_apm_lcm(M_APM r, M_APM u, M_APM v)
{
  M_APM tmpU = M_get_stack_var();

  m_apm_gcd(r, u, v);
  m_apm_integer_divide(tmpU, u, r);
  m_apm_multiply(r, tmpU, v);

  M_restore_stack(1);
}
