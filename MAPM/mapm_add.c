#include "m_apm.h"

static M_APM M_add = NULL;

/****************************************************************************/
void M_free_all_add()
{
    if (M_add == NULL) return;
    m_apm_free(M_add);
    M_add = NULL;
}
/****************************************************************************/
void m_apm_add(M_APM r, M_APM a, M_APM b)
{
    if (a->m_apm_sign + b->m_apm_sign)
      M_add_samesign(r, a, b);
    else
      M_sub_samesign(r, a, b);
}
/****************************************************************************/
void m_apm_subtract(M_APM r, M_APM a, M_APM b)
{
    if (a->m_apm_sign + b->m_apm_sign)
      M_sub_samesign(r, a, b);
    else
      M_add_samesign(r, a, b);
}
/****************************************************************************/

// r = sign(a) * (abs(a) + abs(b))
// if a == 0, r = b

void M_add_samesign(M_APM r, M_APM a, M_APM b)
{
    int sign = a->m_apm_sign;
    if (a->m_apm_sign == 0) {m_apm_copy(r, b); return;}
    if (b->m_apm_sign == 0) {m_apm_copy(r, a); return;}

    int delta = b->m_apm_exponent - a->m_apm_exponent;
    if (delta < 0) {delta = -delta; SWAP(M_APM, a, b);}

    int len = b->m_apm_datalength;
    m_apm_copy(r, a);   // pick the "smaller" one

    if (delta) M_apm_scale(r, delta);
    if (r->m_apm_datalength < len) M_apm_pad(r, 0, len);

    len = (len - 1) >> 1;
    int bcd = 0;
    UCHAR *rp = r->m_apm_data;
    UCHAR *bp = b->m_apm_data;

    do {
        bcd += rp[len] + bp[len];
        rp[len] = bcd = M_bcd_100[bcd];
        bcd >>= 8;
    } while (--len >= 0);

    if (bcd) {          // fix overflow
        M_apm_scale(r, 1);
        r->m_apm_data[0] += 10;
    }
    r->m_apm_sign = sign;
    M_apm_normalize(r);
}

/****************************************************************************/

// r = sign(a) * (abs(a) - abs(b))
// if a == 0, r = -b

void M_sub_samesign(M_APM r, M_APM a, M_APM b)
{
    if (M_add == NULL) M_add = m_apm_init();

    int sign = a->m_apm_sign;
    if (a->m_apm_sign == 0) {m_apm_negate(r, b);return;}
    if (b->m_apm_sign == 0) {m_apm_copy(r, a)  ;return;}

    switch (m_apm_compare_absolute(a, b)) {
        case  0: M_set_to_zero(r); return;
        case -1: SWAP(M_APM, a, b);     // do -b + a
                 sign = -sign;
    }

    int delta = a->m_apm_exponent - b->m_apm_exponent;
    if (delta) {
        m_apm_copy(M_add, b);
        M_apm_scale(M_add, delta);
        b = M_add;
    }

    int len = b->m_apm_datalength;
    m_apm_copy(r, a);   // pick the "bigger" one
    if (r->m_apm_datalength < len) M_apm_pad(r, 0, len);

    len = (len - 1) >> 1;
    int bcd = 1;
    UCHAR *rp = r->m_apm_data;
    UCHAR *bp = b->m_apm_data;

    do {
        bcd += 99 + rp[len] - bp[len];
        rp[len] = bcd = M_bcd_100[bcd];
        bcd >>= 8;
    } while (--len >= 0);

    r->m_apm_sign = sign;
    M_apm_normalize(r);
}
