#include "m_apm.h"
#include "mapm_np2.c"   // next powers of 2
/*
 * FFT_MUL = 0 -> div-and-conq ONLY
 * FFT_MUL = 1 -> fft mul ONLY, fatal if overflow
 * FFT_MUL = 2 -> fft mul AND div-and-conq
 */

/*
 * MAX_FFT_BYTES *must* be an exact power of 2
 *
 * **WORST** case input numbers (all 9's) has shown that
 * the FFT math will overflow if the MAX_FFT_BYTES >= 1048576
 *
 * the define below allows the FFT algorithm to multiply two
 * 524,288 digit numbers yielding a 1,048,576 digit result.
 */

#if FFT_MUL
    #define MAX_FFT_BYTES 524288
    void M_fast_mul_fft(UCHAR *, UCHAR *, UCHAR *, int);
#else
    #define MAX_FFT_BYTES 8             /* NOT using FFT */
    #define M_fast_mul_fft(r,a,b,sz)    M_4_byte_mul(r, a, b)
    static void M_4_byte_mul(UCHAR *, UCHAR *, UCHAR *);
#endif
#define DATA(r)   r->m_apm_data

#if FFT_MUL != 1
void          M_fmul_div_conq(UCHAR *, UCHAR *, UCHAR *, int);
static void   M_fmul_dc2(UCHAR *, UCHAR *, UCHAR *, UCHAR *, int);
static void   M_fmul_add(UCHAR *, UCHAR *, UCHAR *, UCHAR *, int);
static int    M_fmul_sub(UCHAR *, UCHAR *, UCHAR *, int);
#endif

static M_APM  M_ain, M_bin, M_fmul = NULL;

/****************************************************************************/

void M_fast_multiply(M_APM rr, M_APM aa, M_APM bb)
{
    if (M_fmul == NULL) {
        M_fmul = m_apm_init();
        M_ain = m_apm_init();
        M_bin = m_apm_init();
    }

    // Round up to the next power of 2
    // bb->m_apm_datalength >= aa->m_apm_datalength >= 5
    int n = next_pow2(bb->m_apm_datalength);
    if (n > rr->m_apm_malloclength) M_realloc(rr, n + 28);

    m_apm_copy(M_ain, aa);
    m_apm_copy(M_bin, bb);
    M_apm_pad (M_ain, 0, n);
    M_apm_pad (M_bin, 0, n);

#if FFT_MUL
    if (n > MAX_FFT_BYTES)
        #if FFT_MUL == 1
        M_apm_error(M_APM_FATAL, "M_fast_mul_fft overflow");
        #else
        M_fmul_div_conq(DATA(rr), DATA(M_ain), DATA(M_bin), n);
        #endif
    else
        M_fast_mul_fft (DATA(rr), DATA(M_ain), DATA(M_bin), n>>1);
#else
    M_fmul_div_conq(DATA(rr), DATA(M_ain), DATA(M_bin), n);
#endif
    rr->m_apm_sign       = aa->m_apm_sign * bb->m_apm_sign;
    rr->m_apm_exponent   = aa->m_apm_exponent + bb->m_apm_exponent;
    rr->m_apm_datalength = aa->m_apm_datalength + bb->m_apm_datalength;
    M_apm_normalize(rr);
}

/****************************************************************************/
/*
 * Divide-and-Conquer Algorithm:
 *
 * Assume we have 2 numbers (a & b) with 2N decimal digits.
 *
 * let : a = (10^N) * A1 + A0 , b = (10^N) * B1 + B0
 *
 * Now use the identity:
 *
 *           2N    N             N                     N
 * ab  =  (10  + 10 ) A1B1  +  10 (A1-A0)(B0-B1)  + (10 + 1)A0B0
 *
 * Original problem of multiplying 2 (2N) digit numbers has
 * been reduced to 3 multiplications of N digit numbers
 * plus some additions, subtractions, and shifts.
 *
 * The fast multiplication algorithm used here uses the above
 * identity in a recursive process. This algorithm results in
 * O(n ^ 1.585) growth.
 */

#if FFT_MUL != 1

void M_fmul_div_conq(UCHAR *rr, UCHAR *aa, UCHAR *bb, int n)
{
    int sz = n >> 1;
    n -= MAX_FFT_BYTES;     // less than size of rr !
    if (n > M_fmul->m_apm_malloclength) M_realloc(M_fmul, n);
    M_fmul_dc2(rr, aa, bb, DATA(M_fmul), sz);
}

void M_fmul_dc2(UCHAR *rr, UCHAR *a1, UCHAR *b1, UCHAR *cc, int sz)
{
    if (sz < MAX_FFT_BYTES) {M_fast_mul_fft(rr, a1, b1, sz); return;}

    int mi = sz>>1;
    UCHAR* a0 = a1 + mi;
    UCHAR* b0 = b1 + mi;

    // cc = (a1-a0) (b0-b1)
    int sign = M_fmul_sub(rr, a1, a0, mi);
    if (sign) sign *= M_fmul_sub(rr + mi, b0, b1, mi);
    if (sign) M_fmul_dc2(cc, rr, rr + mi, cc + sz, mi);
    if (sign == 0) memset(cc, 0, sz);
    if (sign == -1) {       // build cc complement
      uint32_t *u = (uint32_t *) cc;
      for(int n=sz>>2; n--; u++) *u = 0x63636363 - *u;
      cc[0] -= 100;         // add sign here
      cc[sz-1] += 1;        // -12:-34:-99 => -13:65:01
    }

    // rr = 10^(2N) a1b1 + a0b0 + 10^N (a1b1 + a0b0 + cc)
    M_fmul_dc2(rr,      a1, b1, cc + sz, mi);
    M_fmul_dc2(rr + sz, a0, b0, cc + sz, mi);
    M_fmul_add(rr + mi, rr, rr + sz, cc, sz);
}

/****************************************************************************/

void M_fmul_add(UCHAR *r0, UCHAR *r1, UCHAR *r2, UCHAR *c, int sz)
{
    int i = sz, bcd = 0;    // note: r's are in same area
    while (--i >= 0) {      // note: c[0] might be negative
        bcd += r0[i] + r1[i] + r2[i] + (char) c[i];
        c[i] = bcd = M_bcd_100[bcd];
        bcd >>= 8;
    }
    memcpy(r0, c, sz);
    if (bcd == 0) return;   // no carry
    r0[i] += bcd - 1;       // 1+ carries
    while (++r0[i] >= 100) r0[i--] -= 100;
}

/****************************************************************************/

int M_fmul_sub(UCHAR *r, UCHAR *a, UCHAR *b, int sz)
{
    int i=0, sign=1, bcd=1;
    while(a[i] == b[i])
        if (++i == sz) return 0;    // a == b

    if (a[i] < b[i]) {SWAP(UCHAR*, a, b); sign = -1;}

    for(i = sz; --i >= 0; ) {
        bcd += 99 + a[i] - b[i];
        r[i] = bcd = M_bcd_100[bcd];
        bcd >>= 8;
    }
    return sign;
}

/****************************************************************************/

void M_free_all_fmul()
{
    if (M_fmul == NULL) return;
    m_apm_free(M_fmul);
    M_fmul = NULL;
    m_apm_free(M_ain);
    m_apm_free(M_bin);
}

/****************************************************************************/

#if FFT_MUL == 0
void M_4_byte_mul(UCHAR *r, UCHAR *a, UCHAR *b)
{
    unsigned a0 = 100 * a[0] + a[1];
    unsigned a1 = 100 * a[2] + a[3];
    unsigned b0 = 100 * b[0] + b[1];
    unsigned b1 = 100 * b[2] + b[3];
    unsigned head = a0 * b0;
    unsigned tail = a1 * b1;
    unsigned cross = a0 * b1 + a1 * b0;

    a0 = tail / 10000;
    b1 = tail - a0 * 10000;
    a0 += cross;
    a1 = a0 / 10000;
    b0 = a0 - a1 * 10000;
    a1 += head;
    a0 = a1 / 10000;
    a1 -= a0 * 10000;

    a0 = M_bcd_100[a0];
    a1 = M_bcd_100[a1];
    b0 = M_bcd_100[b0];
    b1 = M_bcd_100[b1];

    r[0] = a0 >> 8; r[1] = a0;
    r[2] = a1 >> 8; r[3] = a1;
    r[4] = b0 >> 8; r[5] = b0;
    r[6] = b1 >> 8; r[7] = b1;
}
#endif
#endif

/****************************************************************************/

void m_apm_trim_mem_usage()
{
#if FFT_MUL
    M_free_all_fft();
#endif
#if FFT_MUL != 1
    M_free_all_fmul();
#endif
    M_free_all_add();
    M_free_all_div();
    M_free_all_rnd();
    M_free_all_stck();
}

void m_apm_free_all_mem()
{
    m_apm_trim_mem_usage();
    M_free_all_util();
    M_free_all_cnst();
}
