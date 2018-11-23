#include "m_apm.h"

static M_APM M_div_a;
static M_APM M_div_b;
static M_APM M_div_c;
static M_APM M_div_r;
static int M_div_firsttime = TRUE;

/****************************************************************************/
void M_free_all_div()
{
    if (M_div_firsttime) return;
    M_div_firsttime = TRUE;
    m_apm_free(M_div_a);
    m_apm_free(M_div_b);
    m_apm_free(M_div_c);
    m_apm_free(M_div_r);
}
/****************************************************************************/
void M_apm_divide(M_APM r, int places, M_APM a, M_APM b)
{
    if (M_div_firsttime) {
        M_div_firsttime = FALSE;
        M_div_a = m_apm_init();
        M_div_b = m_apm_init();
        M_div_c = m_apm_init();
        M_div_r = m_apm_init();
    }

    r->m_apm_sign = a->m_apm_sign * b->m_apm_sign;
    if (r->m_apm_sign == 0) {
        if (b->m_apm_sign == 0)
            M_apm_error(M_APM_RETURN, "M_apm_divide, Divide by 0");
        M_set_to_zero(r);
        return;
    }

    // Scaling to get better estimate of quotient digit q

    unsigned b0 = 100 * b->m_apm_data[0] + 1;
    if (b->m_apm_datalength > 2) b0 += b->m_apm_data[1];
    b0 = 100000 / b0;

    M_mul_digit(M_div_a, a, b0);
    M_mul_digit(M_div_b, b, b0);

    // setup trial denominator using 2 digts (base-100)

    b0 = 100 * M_div_b->m_apm_data[0];
    if (M_div_b->m_apm_datalength > 2) b0 += M_div_b->m_apm_data[1];
    unsigned scaled_b0[] = {100 * b0, 10 * b0, b0};

    // line up exponents so q is 10 to 99 on first divide

    r->m_apm_exponent = M_div_a->m_apm_exponent - M_div_b->m_apm_exponent + 1;
    M_div_a->m_apm_exponent = M_div_b->m_apm_exponent = 0;
    M_div_a->m_apm_sign = 1;

    switch(m_apm_compare_absolute(M_div_b, M_div_a)) {
        case 1: M_div_a->m_apm_exponent = 2;  // a *= 100
                r->m_apm_exponent--;
                break;
        case 0: r->m_apm_data[0] = 10;        // ratio = 1
                r->m_apm_datalength = 1;
                return;
        default: M_div_a->m_apm_exponent++;   // a *= 10
    }

    int n = (places + 5) >> 1;
    r->m_apm_datalength = n * 2;    // skip round-to-even

    if (n > r->m_apm_malloclength) M_realloc(r, n + 28);
    UCHAR *data = r->m_apm_data;
    data[--n] = 0;                  // trailing zero

    for (;;) {

        // trial numerator using 3 base-100 digits

        unsigned a0 = 10000 * M_div_a->m_apm_data[0];

        if (M_div_a->m_apm_datalength > 4) {
            a0 += 100 * M_div_a->m_apm_data[1] + M_div_a->m_apm_data[2];
        }
        else if (M_div_a->m_apm_datalength > 2) {
            a0 += 100 * M_div_a->m_apm_data[1];
        }

        // Since the library 'normalizes' all the results,
        // we need to look at the exponent of the number
        // to decide if we have a lead in 0n or 00.

        unsigned q = a0 / scaled_b0[M_div_a->m_apm_exponent];

        switch (q) {
          case 0: *data++ = 0;  // must be right
                  if (--n == 0) return;
                  M_div_a->m_apm_exponent = 2;
                  q = a0 / b0;  // next q
                  break;
          case 100: q--;        // must be wrong
        }

        // From www.brinch-hansen.net/papers/1994b.pdf
        // Multiple-Length Division Revisited, A Tour of the Minfield
        // Proved q is either correct, or 1 too big
        // Probability of wrong guess < 1 / M_div_b->m_apm_data[0]

        M_mul_digit(M_div_c, M_div_b, q);
        M_sub_samesign(M_div_r, M_div_a, M_div_c);

        if (M_div_r->m_apm_sign < 0) {    // q is too high
            SWAP(M_APM, M_div_r, M_div_c);
            M_sub_samesign(M_div_r, M_div_c, M_div_b);
            q--;
        }
        *data++ = q;

        if (M_div_r->m_apm_sign == 0) {
            r->m_apm_datalength -= 2 * n;
            M_apm_normalize(r);     // possible round-to-even
            return;                 // exact divide
        }

        for(;;) {
          if (--n == 0) return;
          if ((M_div_r->m_apm_exponent += 2) >= 0) break;
          *data++ = 0;
        }
        SWAP(M_APM, M_div_r, M_div_a);
    }
}
/****************************************************************************/
void m_apm_integer_divide(M_APM r, M_APM a, M_APM b)
{
  switch (m_apm_compare_absolute(b, a)) {
    case 1 : M_set_to_zero(r); return;
    case 0 : m_apm_set_long(r, a->m_apm_sign * b->m_apm_sign);
             if (b->m_apm_sign) return; // ok if not div by 0
  }
  int places = a->m_apm_exponent - b->m_apm_exponent;
  M_apm_divide(r, places, a, b);
  if ((unsigned) r->m_apm_datalength > (unsigned) r->m_apm_exponent)
    r->m_apm_datalength = r->m_apm_exponent;
  M_apm_normalize(r);                   // removed fractions
}
/****************************************************************************/
void m_apm_integer_div_rem(M_APM q, M_APM r, M_APM a, M_APM b)
{
    m_apm_integer_divide(q, a, b);
    if (q->m_apm_sign == 0) {m_apm_copy(r, a); return;}

    M_APM tmp = M_get_stack_var();
    m_apm_multiply(tmp, q, b);
    m_apm_subtract(r, a, tmp);
    M_restore_stack(1);
}
