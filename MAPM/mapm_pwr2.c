#include "m_apm.h"

void m_apm_integer_pow_nr(M_APM r, M_APM x, unsigned n)
{
  if (n < 2) {m_apm_copy(r, n ? x : MM_One); return;}

  M_APM t1 = M_get_stack_var();
  m_apm_integer_pow_nr(t1, x, n>>1);
  if (n & 1) {
    M_APM t2 = M_get_stack_var();
    m_apm_multiply(t2, x, t1);
    m_apm_multiply(r, t1, t2);
    M_restore_stack(2);
    return;
  }
  m_apm_square(r, t1);
  M_restore_stack(1);
}

/****************************************************************************/

void m_apm_ishift(M_APM r, int bits)  // r *= 2 ** bits
{
  if (bits & 1) M_mul_digit(r, r, 2);
  if ((bits >>= 1) == 0) return;
  M_APM x = M_get_stack_var();
  M_APM y = M_get_stack_var();

  m_apm_integer_pow_nr(x, (bits>0 ? MM_Two : MM_0_5), abs(bits));
  m_apm_multiply(y, r, x);
  m_apm_multiply(r, x, y);
  M_restore_stack(2);
}
