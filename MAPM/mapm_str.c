#include "m_apm.h"
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

// places > -1 -> round to places , pad with zeroes
// places = -1 -> show all digits , no padding
// places < -1 -> round to -places, no padding

void m_apm_to_string(char *s, int places, M_APM x)
{
  M_APM r = M_get_stack_var();
  m_apm_round(r, places, x);

  if (r->m_apm_sign < 0) *s++ = '-';
  int len = r->m_apm_datalength;
  int exp = r->m_apm_sign ? (r->m_apm_exponent - 1) : 0;
  sprintf(M_to_data_str(s+1, r, len, places+1), "E%+d", exp);
  s[0] = s[1];
  s[1] = '.';
  M_restore_stack(1);
}

void m_apm_to_integer_string(char *s, M_APM x)
{
    // if |input| < 1, result is "0"
    int exp = x->m_apm_exponent;
    if (exp <= 0) {*s++='0'; *s=0; return;}

    if (x->m_apm_sign < 0) *s++ = '-';
    int len = x->m_apm_datalength;

    M_to_data_str(s, x, min(exp, len), exp);
}

void m_apm_to_fixpt_string(char *s, int places, M_APM x)
{
  M_APM r = M_get_stack_var();
  m_apm_copy(r, x);
  if (places != -1) m_apm_iround_fixpt(r, abs(places));

  if (r->m_apm_sign < 0) *s++ = '-';
  int exp = r->m_apm_sign ? r->m_apm_exponent : 1;
  int idx = (exp > 0) ? 1 : 2 - exp;
  int len = r->m_apm_datalength;

  places = (places >= 0) ? places + exp : max(len, exp);
  M_to_data_str(s + idx, r, len, places);

  if (exp > 0) {
    memmove(s, s + idx, exp);     // shift integer part
    s[exp] = s[exp+1] ? '.' : 0;  // do not return ddd.
  } else {
    memset(s++, '0', idx);        // add leading zeroes
    *s = '.';                     // will be 0.dddd ...
  }
  M_restore_stack(1);
}

void m_apm_to_fixpt_stringex(char *s, int places, M_APM x,
                             char dec, char sep, int cnt)
{
  m_apm_to_fixpt_string(s, places, x);
  int n = sep && cnt;
  if (!dec & !n) return;

  if (*s == '-') s++; // skip sign
  char *p = s++;      // look for dec pt

  if (*s && *s != '.') {        // not here ?
    s = p + x->m_apm_exponent;  // should be here
    if (*s && *s != '.') s++;   // maybe rounded up
  }
  if (dec && *s) *s = dec;      // leave integer alone
  if (n) n = (s-p-1) / cnt;     // number of sep char(s)
  if (n <= 0) return;           // no sep needed

  // move fractional parts away
  p = (char *) memmove(s + n, s, strlen(s) + 1);

  // copy backwards, add n sep's
  s--; p--;
  for(int i  ; n--; *p-- = sep)
    for(i=cnt; i--; *p-- = *s--) ;
}

int M_fixpt_bytes(M_APM x, int places, int cnt)
{
  int bytes = 32;
  if (x->m_apm_sign) {
    int exp = x->m_apm_exponent;
    int len = x->m_apm_datalength;
    if (exp > 0) {
      bytes += (places != -1) ? abs(places) + exp : max(len, exp);
      if (cnt > 0) bytes += exp / cnt;
    } else
      bytes += (places != -1) ? abs(places) : len - exp;
  }
  return bytes;
}

char *m_apm_to_fixpt_stringexp(int places, M_APM x,
                               char dec, char sep, int cnt)
{
  char *s = (char *) MAPM_MALLOC(M_fixpt_bytes(x, places, cnt));
  if (s)
    m_apm_to_fixpt_stringex(s, places, x, dec, sep, cnt);
  else
    M_apm_error(M_APM_FATAL, "m_apm_to_fixpt_stringexp, Out of memory");
  return s;
}

// convert 1st len decimals of r->m_apm_data to string
// Assume 0 <= len <= r->m_apm_datalength
// pad with zeroes if not enough digits
// return ptr to end of string

char* M_to_data_str(char *s, M_APM r, int len, int digits)
{
  char *end = s + len;
  UCHAR *data = r->m_apm_data;

  while (s < end) {
    unsigned d = *data++;
    *s++ = d / 10 + '0';
    *s++ = d % 10 + '0';
  }
  if (digits > len) {       // pad with zeroes
    memset(end, '0', digits - len);
    end += digits - len;
  }
  *end = 0;
  return end;
}

// places relative to decimal point (no exponent)
// Mode = round to nearest, half-way to even
// If number = 123.456
// places   return
// 0        123
// 1        123.5
// -1       120
// -2       100
// <= -3    0
//
// Handle case for ZERO without sign check
//
// round-to-even sometimes == round-to-0:
// 500 to -3 places (closest thousands) -> 0

void m_apm_iround_fixpt(M_APM r, int places)
{
  int exp = r->m_apm_exponent;
  if (exp > -places) {m_apm_iround(r, places + exp - 1); return;}

  if (exp == -places) {           // might round-up
    if (r->m_apm_data[0] > 50 ||
       (r->m_apm_data[0] == 50 && r->m_apm_datalength > 1)) {
      r->m_apm_data[0] = 10;      // round-up to 10 ^ exp
      r->m_apm_datalength = 1;
      r->m_apm_exponent++;
      return;
    }
  }
  M_set_to_zero(r);
}
