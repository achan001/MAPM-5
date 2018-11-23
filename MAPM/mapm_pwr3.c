#include "m_apm.h"
// z = x ^ n % m
// Assumed x, n, m all integers, n >= 0

void m_apm_powmod(M_APM z, M_APM x, M_APM n, M_APM m)
{
  m_apm_copy(z, MM_One);
  if (n->m_apm_exponent <= 0) return;

  M_APM bits = M_get_stack_var();
  bits->m_apm_datalength = 0;   // all bits to 0
  M_apm_pad(bits, 0, 7 * n->m_apm_exponent);
  UCHAR *bit = bits->m_apm_data;
  M_APM t = M_get_stack_var();  // decompose n to bits
  m_apm_copy(t, n);
  for(;; bit++) {
    M_mul_digit(t, t, 50);
    if (t->m_apm_datalength <= (t->m_apm_exponent -= 2)) continue;
    if(*bit = 1, --t->m_apm_datalength == 0) break;
    M_apm_normalize(t);         // remove 0.5
  }

  M_APM z0 = z;                 // pointer to real z
  M_APM z1 = M_get_stack_var(); // next value of z
  M_APM mm = M_get_stack_var(); // == 1/m
  m_apm_reciprocal(mm, m->m_apm_exponent + x->m_apm_exponent, m);
  do {
    m_apm_square(z1, z);        // build x ^ bits_of_n % m
    if (*bit) {SWAP(M_APM, z1, z); m_apm_multiply(z1, x, z);}
    if (m_apm_compare_absolute(z1, m) < 0) {SWAP(M_APM, z1, z); continue;}
    m_apm_multiply(z, z1, mm);  // approx quotient
    m_apm_iround(z, z->m_apm_exponent - 1);
    m_apm_multiply(t, z, m);
    m_apm_subtract(z, z1, t);   // approx modulo m
  } while (bit-- > bits->m_apm_data);
  while (z->m_apm_sign == -1 || m_apm_compare_absolute(z, m) > 0) {
    M_sub_samesign(z1, z, m);   // Goal: 0 <= z < |m|
    SWAP(M_APM, z1, z);
  }
  if (z0 != z) m_apm_copy(z0, z);
  M_restore_stack(4);
}
