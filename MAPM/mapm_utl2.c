#include "m_apm.h"

int m_apm_compare_absolute(M_APM ltmp, M_APM rtmp)
{
    if (ltmp->m_apm_exponent > rtmp->m_apm_exponent) return 1;
    if (ltmp->m_apm_exponent < rtmp->m_apm_exponent) return -1;

    /* exponents are the same, check the data */

    int llen = ltmp->m_apm_datalength;
    int rlen = rtmp->m_apm_datalength;
    int less = (llen < rlen);
    int bytes = (1 + (less ? llen : rlen)) >> 1;

    for (int i=0; i < bytes; i++) {
        if (ltmp->m_apm_data[i] > rtmp->m_apm_data[i]) return 1;
        if (ltmp->m_apm_data[i] < rtmp->m_apm_data[i]) return -1;
    }
    return (llen > rlen) - less;
}
/****************************************************************************/
int m_apm_compare(M_APM ltmp, M_APM rtmp)
{
    int lsign = ltmp->m_apm_sign;
    int rsign = rtmp->m_apm_sign;

    if (lsign != rsign) return 2 * (lsign > rsign) - 1;
    if (lsign == 0) return 0;
    return lsign * m_apm_compare_absolute(ltmp, rtmp);
}
/****************************************************************************/
void m_apm_copy(M_APM dest, M_APM src)
{
    int n = (src->m_apm_datalength + 1) >> 1;
    if (n > dest->m_apm_malloclength) M_realloc(dest, n + 28);
    memcpy(dest->m_apm_data, src->m_apm_data, n);
    dest->m_apm_datalength = src->m_apm_datalength;
    dest->m_apm_exponent   = src->m_apm_exponent;
    dest->m_apm_sign       = src->m_apm_sign;
}
/****************************************************************************/
void M_apm_error(int fatal, const char *message)
{
    if (fatal) {
        fprintf(stderr, "MAPM Error: %s\n", message);
        exit(100);
    }
    fprintf(stderr, "MAPM Warning: %s\n", message);
}
