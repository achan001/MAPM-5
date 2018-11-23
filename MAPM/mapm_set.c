#include "m_apm.h"

int M_ilog10(unsigned n)
{
  if (n >= 100000000) return 8 + (n>=1000000000);
  if (n >= 1000000)   return 6 + (n>=10000000);
  if (n >= 10000)     return 4 + (n>=100000);
  if (n >= 100)       return 2 + (n>=1000);
  return (n >= 10);   // ilog10(0) undefined
}
/****************************************************************************/
void M_set_samesign(M_APM r, unsigned n)
{
  int i = M_ilog10(n);
  if (n==0) {M_set_to_zero(r); return;}

  int bytes = i >> 1;
  r->m_apm_exponent = i + 1;
  UCHAR *data = r->m_apm_data + bytes++;

  if ((i & 1)==0) {   // handle odd digit
    *data-- = (n % 10) * 10;
    n /= 10;
  }
  if (i) {            // handle XX digits
    for(; i > 2; i-=2, n/=100) *data-- = n%100;
    *data-- = n;
  }
  while(data[bytes]==0) bytes--;
  r->m_apm_datalength = 2 * bytes - (data[bytes] % 10U == 0);
}
/****************************************************************************/
void m_apm_set_unsigned(M_APM r, unsigned n)
{
  r->m_apm_sign = 1;
  M_set_samesign(r, n);
}
/****************************************************************************/
void m_apm_set_long(M_APM r, int n)
{
  r->m_apm_sign = 1 | (n >> 31);
  M_set_samesign(r, r->m_apm_sign * n);
}
/****************************************************************************/
void m_apm_set_uint64_t(M_APM r, uint64_t n)
{
  r->m_apm_sign = 1;
  if (n <= 0xffffffff) {M_set_samesign(r, n); return;}

  UCHAR *data = r->m_apm_data;
  r->m_apm_exponent = r->m_apm_datalength = 20;

  // n = 2^47 n2 + 2^30 n1 + n0
  //   = 140,7374,8835,5328 n2 + 10,7374,1824 n1 + n0

  unsigned q, n0, n1, n2;   // 64bit = 17 + 17 + 30
  q  = n & 0x3fffffff;
  n1 = (n >> 30) & 0x1ffff;
  n2 = (n >> 47);

  q += 5328*n2 + 1824*n1;
  data[9] = n0 = M_bcd_100[q % 10000];
  data[8] = n0>>8;

  q = q/10000 + 8835*n2 + 7374*n1;
  data[7] = n0 = M_bcd_100[q % 10000];
  data[6] = n0>>8;

  q = q/10000 + 7374*n2 + 10*n1;
  data[5] = n0 = M_bcd_100[q % 10000];
  data[4] = n0>>8;

  q = q/10000 + 140*n2;
  data[3] = n0 = M_bcd_100[q % 10000];
  data[2] = n0>>8;
  data[1] = n0 = M_bcd_100[q / 10000];
  data[0] = n0>>8;
  M_apm_normalize(r);
}
/****************************************************************************/
void M_set_string(M_APM r, const char *str, char **end)
{
  const char *s, *p = str;
  int digits=0, sign=1, d=0, i=-1;

  while(ISSPACE(*p)) p++;

  if      (*p == '-') p++, sign = -1;
  else if (*p == '+') p++;
  r->m_apm_sign = sign;

  for(s=p; ; p++) {         // parse decimal string
    if (ISDIGIT(*p)) continue;
    if (*p == '.' && i < 0) {i = p - s; continue;}
    if ((digits = p - s - (i >= 0))) break;
    if (end) *end = (char *) str;
    M_set_to_zero(r);       // bad input
    return;
  }

  if ((*p|32) == 'e') {     // build exponent
    const char *q = p + 1;
    if      (*q == '-') q++, sign = 0;
    else if (*q == '+') q++;
    if (ISDIGIT(*q)) {      // valid exponent
      for(p=q+1,d=*q-'0'; ISDIGIT(*p); p++)
        if (d < 100000000) d = 10 * d + *p - '0';
      if (sign==0) d = -d;
    }
  }
  if (end) *end = (char *) p;

  int n = (digits + 1) >> 1;
  if (n > r->m_apm_malloclength) M_realloc(r, n + 28);

  r->m_apm_datalength = 2 * n;
  UCHAR *data = r->m_apm_data;

  if (i < 0) {i = digits; goto NO_POINT;}   // dddd -> dddd.

  for(n=i>>1; --n >=0 ; s += 2)
    *data++ = 10 * s[0] + s[1] - 11 * '0';

  digits -= i;  // digits after dec pt
  if ((i & 1) == 0) {++s; goto NO_POINT;}   // .dddd -> dddd
  if (!digits) {++digits; goto NO_POINT;}   // d.    -> d

  digits--;     // dec pt between digits
  *data++ = 10 * s[0] + s[2] - 11 * '0';
  s += 3;       // digit + dec pt + digit

  NO_POINT:
  for(n=digits>>1; --n >= 0; s += 2)
    *data++ = 10 * s[0] + s[1] - 11 * '0';

  if (digits & 1) *data = 10 * s[0] - 10 * '0';

  r->m_apm_exponent = d + i;
  M_apm_normalize(r);
}
