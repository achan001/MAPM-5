#include "m_apm.h"

// r = (a b) * ((a+1) (b-1)) ... ((a + n) * (b - n))

void M_mul_edge(M_APM r, int a, int b, int n)
{
  if (n == 0) {m_apm_set_uint64_t(r, (uint64_t) a * b); return;}
  int k = n >> 1;
  M_APM x = M_get_stack_var();    // outer pairs product
  M_mul_edge(x, a, b, k++);
  M_APM y = M_get_stack_var();    // inner pairs product
  M_mul_edge(y, a+k, b-k, n-k);
  m_apm_multiply(r, x, y);        // x, y about same digits
  M_restore_stack(2);
}

void M_factorial(M_APM r, int n)
{
  if (n < 2) {m_apm_copy(r, MM_One);return;}
  M_mul_edge(r, (n & 1) + 1, n, (n>>1) - 1);
}

void m_apm_factorial(M_APM r, M_APM x)
{
  if (x->m_apm_sign < 0 || x->m_apm_exponent > 8 || !m_apm_is_integer(x)) {
    M_apm_error(M_APM_RETURN, "m_apm_factorial, invalid input");
    M_set_to_zero(r);
    return;
  }
  M_factorial(r, lrint(m_apm_to_double(x)));
}
