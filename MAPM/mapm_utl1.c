#include "m_apm.h"

int m_apm_sign(M_APM atmp)
{
    return atmp->m_apm_sign;
}
/****************************************************************************/
int m_apm_exponent(M_APM atmp)
{
    return atmp->m_apm_sign ? (atmp->m_apm_exponent - 1) : 0;
}
/****************************************************************************/
int m_apm_significant_digits(M_APM atmp)
{
    return atmp->m_apm_datalength;
}
/****************************************************************************/
void m_apm_negate(M_APM d, M_APM s)
{
    m_apm_copy(d, s);
    d->m_apm_sign *= -1;
}
/****************************************************************************/
void m_apm_absolute_value(M_APM d, M_APM s)
{
    m_apm_copy(d, s);
    d->m_apm_sign &= 1;
}
/****************************************************************************/
int m_apm_is_integer(M_APM atmp)
{
    return (atmp->m_apm_exponent >= atmp->m_apm_datalength) ||
           (atmp->m_apm_sign == 0);
}
/****************************************************************************/
int m_apm_is_odd(M_APM aa)
{
    int ii = aa->m_apm_datalength;
    int jj = aa->m_apm_exponent;

    if (jj < ii) {
        if (aa->m_apm_sign)
            M_apm_error(M_APM_RETURN, "m_apm_is_odd, Non-integer input");
        return 0 ;
    }

    if (jj > ii) return 0;

    ii = aa->m_apm_data[(ii - 1) >> 1];

    if (jj & 1) ii /= 10U;

    return (ii & 1);
}
