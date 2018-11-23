#include "m_apm.h"

uint16_t *M_bcd_100 = NULL;
static uint16_t M_bcd_10x[100];
static char M_init_error_msg[] = "m_apm_init, Out of memory";

M_APM m_apm_init()
{
  if (M_bcd_100 == NULL) {
    M_init_util_data();
    M_init_all_cnst();
  }

  M_APM r = (M_APM) MAPM_MALLOC(sizeof(M_APM_struct));
  UCHAR *data = (UCHAR *) MAPM_MALLOC(50 + 4);
  
  if (!r || !data) M_apm_error(M_APM_FATAL, M_init_error_msg);
  
  *(r->m_apm_data = data) = 0;  // build MM_Zero
  r->m_apm_id = M_APM_IDENT;
  r->m_apm_refcount = 1;
  r->m_apm_malloclength = 50;
  r->m_apm_datalength = 1;
  r->m_apm_exponent = 1<<31;    // == INT32_MIN
  r->m_apm_sign = 0;
  return r;
}

/****************************************************************************/
void M_set_to_zero(M_APM z)
{
    z->m_apm_sign       = 0;
    z->m_apm_data[0]    = 0;
    z->m_apm_datalength = 1;
    z->m_apm_exponent   = 1<<31;
}
/****************************************************************************/

void M_init_util_data()
{
  if (M_bcd_100 != NULL) return;

  M_bcd_100 = (uint16_t*) MAPM_MALLOC(20000 * sizeof(uint16_t));

  if (!M_bcd_100)
    M_apm_error(M_APM_FATAL, "M_init_util_data, Out of memory");

  int i, k;
  for (k = i = 0; i < 100; i++) {
    M_bcd_10x[i] = k;   // k == (i/10)*256 + (i%10)*10
    if ((UCHAR)(k += 10) == 100) k += 256 - 100;
  }
  for (k = i = 0; i < 20000; i++) {
    M_bcd_100[i] = k;   // k == (i/100)*256 + (i%100)
    if ((UCHAR) ++k == 100) k += 256 - 100;
  }
}

/****************************************************************************/

void M_apm_normalize(M_APM r)
{
  UCHAR *data    = r->m_apm_data;
  int exponent   = r->m_apm_exponent;
  int datalength = r->m_apm_datalength;
  int last       = (datalength - 1) >> 1;

  if (datalength & 1) {             // fix odd digits
    data[last] = 10 * (M_bcd_10x[data[last]] >> 8);
    if (data[last--]) goto LEADING_ZERO;
    if ((datalength -= 1) == 0) {M_set_to_zero(r); return;}
  }

  for(; data[last] == 0; last--) {  // remove trailing zeroes
    if ((datalength -= 2) == 0) {M_set_to_zero(r); return;}
  }
  datalength -= ((UCHAR) M_bcd_10x[data[last]] == 0);

  LEADING_ZERO:

  if (data[0] == 0) {
    int zeroes = 1;
    int nbytes = (datalength + 1) >> 1;
    while (data[zeroes] == 0) zeroes++;
    memmove(data, data + zeroes, nbytes - zeroes);
    zeroes <<= 1;         // 1 byte = 2 zeroes
    exponent -= zeroes;
    datalength -= zeroes;
  }
  if (data[0] < 10) {
    UCHAR rem10x = M_bcd_10x[data[0]];
    int nbytes = (datalength + 1) >> 1;
    datalength--;
    exponent--;
    while(--nbytes) {     // left shift digits
      int bcd = M_bcd_10x[data[1]];
      data++[0] = (bcd >> 8) + rem10x;
      rem10x = bcd;
    }
    data[0] = rem10x;
  }
  r->m_apm_datalength = datalength;
  r->m_apm_exponent   = exponent;
}

/****************************************************************************/

void M_realloc(M_APM r, unsigned n)
{
  void *vp = MAPM_REALLOC(r->m_apm_data, n + 4);
  if (!vp) M_apm_error(M_APM_FATAL, "M_realloc error");
  r->m_apm_malloclength = n;
  r->m_apm_data = (UCHAR *)vp;
}

/****************************************************************************/

void M_apm_scale(M_APM r, int ct)
{
  int n = (r->m_apm_datalength + ct + 1) >> 1;
  int i = r->m_apm_datalength;

  if (n > r->m_apm_malloclength) M_realloc(r, n + 28);
  UCHAR *data = r->m_apm_data;

  if (ct & 1) {       // shift odd number first
    int bcd = 0;
    for(i=(i+1)>>1; i; ) {
      bcd += M_bcd_10x[data[i-1]];
      data[i--] = bcd;
      bcd >>= 8;
    }
    data[0] = bcd;
  }

  i = ct >> 1;
  if (i) {           // shift multiples of 00
    memmove(data + i, data, n - i);
    memset(data, 0, i);
  }
  r->m_apm_datalength += ct;
  r->m_apm_exponent += ct;
}

/****************************************************************************/

void M_apm_pad(M_APM r, int digit, int ct)
{
  if (r->m_apm_datalength >= ct) return;

  int n = (ct + 1) >> 1;
  int i = (r->m_apm_datalength + 1) >> 1;

  if (n > r->m_apm_malloclength) M_realloc(r, n + 28);
  memset(r->m_apm_data + i, digit, n - i);
  r->m_apm_datalength = ct;
}

/****************************************************************************/

void m_apm_iround(M_APM r, int places)
{
  if (places == -1) return;     // no rounding

  places = abs(places);
  if (++places >= r->m_apm_datalength) return;

  UCHAR *data = r->m_apm_data;
  int i = places >> 1;          // index of rounding
  int e = (places & 1) ? 5 : 50;

  if (places + 1 == r->m_apm_datalength) {
    int d = data[(places - 1) >> 1];
    if (e - 50) d = M_bcd_10x[d] >> 8;
    if (~d & 1) e--;            // nudge round-to-even
  }

  data[i] += e;
  while(data[i] >= 100) {       // do carry
    if (i == 0) {
      data[0] = 10;
      r->m_apm_datalength = 1;  // rounding overflow !
      r->m_apm_exponent++;      // r == 1E+xxx
      return;
    }
    data[i--] = 0;
    data[i]++;
  }
  r->m_apm_datalength = places;
  M_apm_normalize(r);
}

/****************************************************************************/

void m_apm_round(M_APM r, int places, M_APM x)
{
  m_apm_copy(r, x);
  m_apm_iround(r, places);
}

/****************************************************************************/

void m_apm_ichop(M_APM r, int places)
{
  if (++places >= r->m_apm_datalength) return;
  r->m_apm_datalength = places;
  M_apm_normalize(r);
}

/****************************************************************************/

void m_apm_free(M_APM r)
{
  if (r->m_apm_id == M_APM_IDENT) {
    r->m_apm_id = ~M_APM_IDENT;
    MAPM_FREE(r->m_apm_data);
    MAPM_FREE(r);
    return;
  }
  M_apm_error(M_APM_RETURN, "m_apm_free, Invalid M_APM variable");
}

/****************************************************************************/

void M_free_all_util()
{
  if (M_bcd_100 == NULL) return;
  MAPM_FREE(M_bcd_100);
  M_bcd_100 = NULL;
}
