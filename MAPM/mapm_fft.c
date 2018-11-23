#include "m_apm.h"

#define HALF_PI     1.5707963267948966
#define SQRT2R      0.7071067811865476
#define RDFT_LOOP   64

#if defined(__GNUC__) && defined(__i386)
#define SINCOS(x,s,c)   asm("fsincos": "=t"(c), "=u"(s): "0"(x))
#else
#define SINCOS(x,s,c)   c=cos(x); s=sin(x)
#endif

static void M_rdft_forward(int, double *);
static void M_rdft_inverse(int, double *);
static int    M_size = 0;
static double *M_aa_array = NULL, *M_bb_array = NULL;
static char   M_fft_error_msg[] = "M_fast_mul_fft, Out of memory";

/****************************************************************************/
void M_free_all_fft()
{
    if (M_size) {
        M_size = 0;
        MAPM_FREE(M_aa_array); M_aa_array = NULL;
        MAPM_FREE(M_bb_array); M_bb_array = NULL;
    }
}
/****************************************************************************/
/*
 *      multiply 'uu' by 'vv' with nbytes each
 *      yielding a 2*nbytes result in 'ww'.
 *      each byte contains a base 100 'digit',
 *      i.e.: range from 0-99.
 *
 *             MSB              LSB
 *
 *   uu,vv     [0] [1] [2] ... [N-1]
 *   ww        [0] [1] [2] ... [2N-1]
 */

void M_fast_mul_fft(UCHAR *ww, UCHAR *uu, UCHAR *vv, int sz)
{
    double *aa, *bb;
    if (sz > M_size) {
        M_size = sz > 8200 ? sz : 8200;
        aa = (double *) MAPM_REALLOC(M_aa_array, M_size * sizeof(double));
        bb = (double *) MAPM_REALLOC(M_bb_array, M_size * sizeof(double));
        if (!aa || !bb) M_apm_error(M_APM_FATAL, M_fft_error_msg);
        M_aa_array = aa;
        M_bb_array = bb;
    } else {
        aa = M_aa_array;
        bb = M_bb_array;
    }

    /* convert MAPM numbers to base 10000 */

    double *a = aa, *b = bb;
    for (int i=0; i < sz; i+=4) {
        *a++ = 100 * uu[i]   + uu[i+1];
        *a++ = 100 * uu[i+2] + uu[i+3];
    }
    for (int i=0; i < sz; i+=4) {
        *b++ = 100 * vv[i]   + vv[i+1];
        *b++ = 100 * vv[i+2] + vv[i+3];
    }

    /* zero fill the second half of the arrays */
    /* hex value of 0.0 == 0ULL */
    /* perform the forward Fourier transforms */

    int i = sz >> 1;
    memset(a, 0, i * sizeof(double));
    M_rdft_forward(sz, a = aa);
    
    memset(b, 0, i * sizeof(double));
    M_rdft_forward(sz, b = bb);

    /* perform the convolution ... */

    *b++ *= *a++;
    *b++ *= *a++;
    for(; --i; a += 2) {
        double b0 = b[0] , b1 = b[1];
        *b++ = b0 * a[0] - b1 * a[1];
        *b++ = b0 * a[1] + b1 * a[0];
    }

    /* perform the inverse transform */

    M_rdft_inverse(sz, b = bb);

    /* perform a final pass to release all the carries */
    /* and convert back from base 10000 to base 100 */
    /* NOTE: 0x1A36E2EBp-42 ~= 1e-4 - 2.5e-14 */

    uint64_t n=0, q=0;    // q = [n/10000 - 2, n/10000]
    double k = 2.0 / sz;  // scaling factor
    ww += 2 * sz - 1;     // fill backwards
    b += sz - 2;          // skip last 0.0
    do {
        n = llrint(k * *b--) + q;
        q = 0x1A36E2EBULL * (n>>10) >> 32;
        i = n - 10000 * q;
        for(; i >= 10000; i -= 10000) q++;
        *ww-- = i = M_bcd_100[i];
        *ww-- = i >> 8;
    } while (b >= bb);
    *ww-- = i = M_bcd_100[q];
    *ww   = i >> 8;
}

/****************************************************************************/
/*
 *    The following info is from Takuya OOURA's documentation :
 *
 *    NOTE : MAPM only uses the 'RDFT' function (as well as the
 *           functions RDFT calls). All the code from here down
 *           in this file is from Takuya OOURA. The only change I
 *           made was to add 'M_' in front of all the functions
 *           I used. This was to guard against any possible
 *           name collisions in the future.
 *
 *    MCR  06 July 2000
 *
 *
 *    General Purpose FFT (Fast Fourier/Cosine/Sine Transform) Package
 *
 *    Description:
 *        A package to calculate Discrete Fourier/Cosine/Sine Transforms of
 *        1-dimensional sequences of length 2^N.
 *
 *        fft4g_h.c  : FFT Package in C       - Simple Version I   (radix 4,2)
 *
 *        rdft: Real Discrete Fourier Transform
 *
 *    Method:
 *        -------- rdft --------
 *        A method with a following butterfly operation appended to "cdft".
 *        In forward transform :
 *            A[k] = sum_j=0^n-1 a[j]*W(n)^(j*k), 0<=k<=n/2,
 *                W(n) = exp(2*pi*i/n),
 *        this routine makes an array x[] :
 *            x[j] = a[2*j] + i*a[2*j+1], 0<=j<n/2
 *        and calls "cdft" of length n/2 :
 *            X[k] = sum_j=0^n/2-1 x[j] * W(n/2)^(j*k), 0<=k<n.
 *        The result A[k] are :
 *            A[k]     = X[k]     - (1+i*W(n)^k)/2 * (X[k]-conjg(X[n/2-k])),
 *            A[n/2-k] = X[n/2-k] +
 *                            conjg((1+i*W(n)^k)/2 * (X[k]-conjg(X[n/2-k]))),
 *                0<=k<=n/2
 *            (notes: conjg() is a complex conjugate, X[n/2]=X[0]).
 *        ----------------------
 *
 *    Reference:
 *        * Masatake MORI, Makoto NATORI, Tatuo TORII: Suchikeisan,
 *          Iwanamikouzajyouhoukagaku18, Iwanami, 1982 (Japanese)
 *        * Henri J. Nussbaumer: Fast Fourier Transform and Convolution
 *          Algorithms, Springer Verlag, 1982
 *        * C. S. Burrus, Notes on the FFT (with large FFT paper list)
 *          http://www-dsp.rice.edu/research/fft/fftnote.asc
 *
 *    Copyright:
 *        Copyright(C) 1996-1999 Takuya OOURA
 *        email: ooura@mmm.t.u-tokyo.ac.jp
 *        download: http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html
 *        You may use, copy, modify this code for any purpose and
 *        without fee. You may distribute this ORIGINAL package.
 */

#define SWAP_2DBL(a,i,j) SWAP(double, a[i], a[j])
#define SWAP_4DBL(a,i,j) {SWAP_2DBL(a,i,j); SWAP_2DBL(a,(i)+1,(j)+1);}

void M_bitrv2(int n, double *a)
{
    int i, j, k, j0=0, k0=0, m=2;
    i = n >> 2;
    n >>= 1;
    while (m < i) {i >>= 1; m <<= 1;}
    if (m == i) {
        for (; k0 < m; k0 += 2) {
            for (k=k0, j=j0; j < j0 + k0; j += 2) {
                SWAP_4DBL(a, j, k);
                SWAP_4DBL(a, j + m, k + 2*m);
                SWAP_4DBL(a, j + 2*m, k + m);
                SWAP_4DBL(a, j + 3*m, k + 3*m);
                for (i=n; i > (k ^= i); i >>= 1) ;
            }
            k = j0 + k0 + m;
            SWAP_4DBL(a, k, k + m);
            for (i=n; i > (j0 ^= i); i >>= 1) ;
        }
    } else {
        for (k0=2; k0 < m; k0 += 2) {
            for (i=n; i > (j0 ^= i); i >>= 1) ;
            for (k=k0, j=j0; j < j0 + k0; j += 2) {
                SWAP_4DBL(a, j, k);
                SWAP_4DBL(a, j + m, k + m);
                for (i=n; i > (k ^= i); i >>= 1) ;
            }
        }
    }
}


void M_cft1st(int n, double *a)
{
    int j, kj, kr;
    double ew, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    x0i = a[1] + a[3];
    x1i = a[1] - a[3];
    x0r = a[0] + a[2];
    x1r = a[0] - a[2];
    x2i = a[5] + a[7];
    x3i = a[5] - a[7];
    x2r = a[4] + a[6];
    x3r = a[4] - a[6];
    a[0] = x0r + x2r;
    a[1] = x0i + x2i;
    a[2] = x1r - x3i;
    a[3] = x1i + x3r;
    a[4] = x0r - x2r;
    a[5] = x0i - x2i;
    a[6] = x1r + x3i;
    a[7] = x1i - x3r;
    x0i = a[9] + a[11];
    x1i = a[9] - a[11];
    x0r = a[8] + a[10];
    x1r = a[8] - a[10];
    x2i = a[13] + a[15];
    x3i = a[13] - a[15];
    x2r = a[12] + a[14];
    x3r = a[12] - a[14];
    a[8] = x0r + x2r;
    a[9] = x0i + x2i;
    a[12] = x2i - x0i;
    a[13] = x0r - x2r;
    x0r = x1r - x3i;
    x0i = x1i + x3r;
    a[10] = SQRT2R * (x0r - x0i);
    a[11] = SQRT2R * (x0r + x0i);
    x0r = x3i + x1r;
    x0i = x3r - x1i;
    a[14] = SQRT2R * (x0i - x0r);
    a[15] = SQRT2R * (x0i + x0r);
    ew = HALF_PI / n;
    kr = 0;
    for (j = 16; j < n; j += 16) {
        for (kj = n >> 2; kj > (kr ^= kj); kj >>= 1);
        SINCOS(ew * kr, wk1i, wk1r);
        wk2i = 2 * wk1i * wk1r;
        wk2r = 1 - 2 * wk1i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        wk3r = wk1r - 2 * wk2i * wk1i;
        x0i = a[j + 1] + a[j + 3];
        x1i = a[j + 1] - a[j + 3];
        x0r = a[j] + a[j + 2];
        x1r = a[j] - a[j + 2];
        x2i = a[j + 5] + a[j + 7];
        x3i = a[j + 5] - a[j + 7];
        x2r = a[j + 4] + a[j + 6];
        x3r = a[j + 4] - a[j + 6];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 4] = wk2r * x0r - wk2i * x0i;
        a[j + 5] = wk2r * x0i + wk2i * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 2] = wk1r * x0r - wk1i * x0i;
        a[j + 3] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 6] = wk3r * x0r - wk3i * x0i;
        a[j + 7] = wk3r * x0i + wk3i * x0r;
        x0r = SQRT2R * (wk1r - wk1i);
        wk1i = SQRT2R * (wk1r + wk1i);
        wk1r = x0r;
        wk3i = 2 * wk2r * wk1r - wk1i;
        wk3r = wk1r - 2 * wk2r * wk1i;
        x0i = a[j + 9] + a[j + 11];
        x1i = a[j + 9] - a[j + 11];
        x0r = a[j + 8] + a[j + 10];
        x1r = a[j + 8] - a[j + 10];
        x2i = a[j + 13] + a[j + 15];
        x3i = a[j + 13] - a[j + 15];
        x2r = a[j + 12] + a[j + 14];
        x3r = a[j + 12] - a[j + 14];
        a[j + 8] = x0r + x2r;
        a[j + 9] = x0i + x2i;
        x0r -= x2r;
        x0i -= x2i;
        a[j + 12] = -wk2i * x0r - wk2r * x0i;
        a[j + 13] = -wk2i * x0i + wk2r * x0r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j + 10] = wk1r * x0r - wk1i * x0i;
        a[j + 11] = wk1r * x0i + wk1i * x0r;
        x0r = x1r + x3i;
        x0i = x1i - x3r;
        a[j + 14] = wk3r * x0r - wk3i * x0i;
        a[j + 15] = wk3r * x0i + wk3i * x0r;
    }
}


void M_cftmdl(int n, int l, double *a)
{
    int j, j1, j2, j3, k, kj, kr, m, m2;
    double ew, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    m = l << 2;
    for (j = 0; j < l; j += 2) {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0i = a[j + 1] + a[j1 + 1];
        x1i = a[j + 1] - a[j1 + 1];
        x0r = a[j] + a[j1];
        x1r = a[j] - a[j1];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3i = a[j2 + 1] - a[j3 + 1];
        x2r = a[j2] + a[j3];
        x3r = a[j2] - a[j3];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x0r - x2r;
        a[j2 + 1] = x0i - x2i;
        a[j1] = x1r - x3i;
        a[j1 + 1] = x1i + x3r;
        a[j3] = x1r + x3i;
        a[j3 + 1] = x1i - x3r;
    }
    for (j = m; j < l + m; j += 2) {
        j1 = j + l;
        j2 = j1 + l;
        j3 = j2 + l;
        x0i = a[j + 1] + a[j1 + 1];
        x1i = a[j + 1] - a[j1 + 1];
        x0r = a[j] + a[j1];
        x1r = a[j] - a[j1];
        x2i = a[j2 + 1] + a[j3 + 1];
        x3i = a[j2 + 1] - a[j3 + 1];
        x2r = a[j2] + a[j3];
        x3r = a[j2] - a[j3];
        a[j] = x0r + x2r;
        a[j + 1] = x0i + x2i;
        a[j2] = x2i - x0i;
        a[j2 + 1] = x0r - x2r;
        x0r = x1r - x3i;
        x0i = x1i + x3r;
        a[j1] = SQRT2R * (x0r - x0i);
        a[j1 + 1] = SQRT2R * (x0r + x0i);
        x0r = x3i + x1r;
        x0i = x3r - x1i;
        a[j3] = SQRT2R * (x0i - x0r);
        a[j3 + 1] = SQRT2R * (x0i + x0r);
    }
    ew = HALF_PI / n;
    kr = 0;
    m2 = 2 * m;
    for (k = m2; k < n; k += m2) {
        for (kj = n >> 2; kj > (kr ^= kj); kj >>= 1);
        SINCOS(ew * kr, wk1i, wk1r);
        wk2i = 2 * wk1i * wk1r;
        wk2r = 1 - 2 * wk1i * wk1i;
        wk3i = 2 * wk2i * wk1r - wk1i;
        wk3r = wk1r - 2 * wk2i * wk1i;
        for (j = k; j < l + k; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0i = a[j + 1] + a[j1 + 1];
            x1i = a[j + 1] - a[j1 + 1];
            x0r = a[j] + a[j1];
            x1r = a[j] - a[j1];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3i = a[j2 + 1] - a[j3 + 1];
            x2r = a[j2] + a[j3];
            x3r = a[j2] - a[j3];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            a[j2] = wk2r * x0r - wk2i * x0i;
            a[j2 + 1] = wk2r * x0i + wk2i * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            a[j1] = wk1r * x0r - wk1i * x0i;
            a[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            a[j3] = wk3r * x0r - wk3i * x0i;
            a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
        x0r = SQRT2R * (wk1r - wk1i);
        wk1i = SQRT2R * (wk1r + wk1i);
        wk1r = x0r;
        wk3i = 2 * wk2r * wk1r - wk1i;
        wk3r = wk1r - 2 * wk2r * wk1i;
        for (j = k + m; j < l + (k + m); j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0i = a[j + 1] + a[j1 + 1];
            x1i = a[j + 1] - a[j1 + 1];
            x0r = a[j] + a[j1];
            x1r = a[j] - a[j1];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3i = a[j2 + 1] - a[j3 + 1];
            x2r = a[j2] + a[j3];
            x3r = a[j2] - a[j3];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            x0r -= x2r;
            x0i -= x2i;
            a[j2] = -wk2i * x0r - wk2r * x0i;
            a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
            x0r = x1r - x3i;
            x0i = x1i + x3r;
            a[j1] = wk1r * x0r - wk1i * x0i;
            a[j1 + 1] = wk1r * x0i + wk1i * x0r;
            x0r = x1r + x3i;
            x0i = x1i - x3r;
            a[j3] = wk3r * x0r - wk3i * x0i;
            a[j3 + 1] = wk3r * x0i + wk3i * x0r;
        }
    }
}


void M_rftfsub(int n, double *a)
{
    int i, i0, j, k;
    double ec, w1r, w1i, wkr, wki, wdr, wdi, ss, xr, xi, yr, yi;

    ec = 2 * HALF_PI / n;
    wkr = 0;
    wki = 0;
    SINCOS(ec, wdr, wdi);
    wdi *= wdr;
    wdr *= wdr;
    w1i = 2 * wdi;
    w1r = 1 - 2 * wdr;
    ss = 2 * w1i;
    i = n >> 1;
    while (1) {
        i0 = i - 4 * RDFT_LOOP;
        if (i0 < 4) {
            i0 = 4;
        }
        for (j = i - 4; j >= i0; j -= 4) {
            k = n - j;
            xi = a[j + 3] + a[k - 1];
            xr = a[j + 2] - a[k - 2];
            yi = wdr * xi + wdi * xr;
            yr = wdr * xr - wdi * xi;
            a[j + 2] -= yr;
            a[j + 3] -= yi;
            a[k - 2] += yr;
            a[k - 1] -= yi;
            wkr += ss * wdi;
            wki += ss * (0.5 - wdr);
            xi = a[j + 1] + a[k + 1];
            xr = a[j] - a[k];
            yi = wkr * xi + wki * xr;
            yr = wkr * xr - wki * xi;
            a[j] -= yr;
            a[j + 1] -= yi;
            a[k] += yr;
            a[k + 1] -= yi;
            wdr += ss * wki;
            wdi += ss * (0.5 - wkr);
        }
        if (i0 == 4) {
            break;
        }
        SINCOS(ec * i0, wkr, wki);
        wki *= 0.5;
        wkr *= 0.5;
        wdi = wkr * w1i + wki * w1r;
        wdr = 0.5 - (wkr * w1r - wki * w1i);
        wkr = 0.5 - wkr;
        i = i0;
    }
    xi = a[3] + a[n - 1];
    xr = a[2] - a[n - 2];
    yi = wdr * xi + wdi * xr;
    yr = wdr * xr - wdi * xi;
    a[2] -= yr;
    a[3] -= yi;
    a[n - 2] += yr;
    a[n - 1] -= yi;
}


void M_rftbsub(int n, double *a)
{
    int i, i0, j, k;
    double ec, w1r, w1i, wkr, wki, wdr, wdi, ss, xr, xi, yr, yi;

    ec = 2 * HALF_PI / n;
    wkr = 0;
    wki = 0;
    SINCOS(ec, wdr, wdi);
    wdi *= wdr;
    wdr *= wdr;
    w1i = 2 * wdi;
    w1r = 1 - 2 * wdr;
    ss = 2 * w1i;
    i = n >> 1;
    a[i + 1] = -a[i + 1];
    while (1) {
        i0 = i - 4 * RDFT_LOOP;
        if (i0 < 4) {
            i0 = 4;
        }
        for (j = i - 4; j >= i0; j -= 4) {
            k = n - j;
            xi = a[j + 3] + a[k - 1];
            xr = a[j + 2] - a[k - 2];
            yi = wdr * xi - wdi * xr;
            yr = wdr * xr + wdi * xi;
            a[j + 2] -= yr;
            a[j + 3] = yi - a[j + 3];
            a[k - 2] += yr;
            a[k - 1] = yi - a[k - 1];
            wkr += ss * wdi;
            wki += ss * (0.5 - wdr);
            xi = a[j + 1] + a[k + 1];
            xr = a[j] - a[k];
            yi = wkr * xi - wki * xr;
            yr = wkr * xr + wki * xi;
            a[j] -= yr;
            a[j + 1] = yi - a[j + 1];
            a[k] += yr;
            a[k + 1] = yi - a[k + 1];
            wdr += ss * wki;
            wdi += ss * (0.5 - wkr);
        }
        if (i0 == 4) {
            break;
        }
        SINCOS(ec * i0, wkr, wki);
        wki *= 0.5;
        wkr *= 0.5;
        wdi = wkr * w1i + wki * w1r;
        wdr = 0.5 - (wkr * w1r - wki * w1i);
        wkr = 0.5 - wkr;
        i = i0;
    }
    xi = a[3] + a[n - 1];
    xr = a[2] - a[n - 2];
    yi = wdr * xi - wdi * xr;
    yr = wdr * xr + wdi * xi;
    a[2] -= yr;
    a[3] = yi - a[3];
    a[n - 2] += yr;
    a[n - 1] = yi - a[n - 1];
    a[1] = -a[1];
}


void M_cftfsub(int n, double *a)
{
    int j, j1, j2, j3, l;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    l = 2;
    if (n > 8) {
        M_cft1st(n, a);
        for(l=8; ((l << 2) < n); l <<= 2)
            M_cftmdl(n, l, a);
    }
    if ((l << 2) == n) {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0i = a[j + 1] + a[j1 + 1];
            x1i = a[j + 1] - a[j1 + 1];
            x0r = a[j] + a[j1];
            x1r = a[j] - a[j1];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3i = a[j2 + 1] - a[j3 + 1];
            x2r = a[j2] + a[j3];
            x3r = a[j2] - a[j3];
            a[j] = x0r + x2r;
            a[j + 1] = x0i + x2i;
            a[j2] = x0r - x2r;
            a[j2 + 1] = x0i - x2i;
            a[j1] = x1r - x3i;
            a[j1 + 1] = x1i + x3r;
            a[j3] = x1r + x3i;
            a[j3 + 1] = x1i - x3r;
        }
    } else {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            x0i = a[j + 1] - a[j1 + 1];
            x0r = a[j] - a[j1];
            a[j] += a[j1];
            a[j + 1] += a[j1 + 1];
            a[j1] = x0r;
            a[j1 + 1] = x0i;
        }
    }
}


void M_cftbsub(int n, double *a)
{
    int j, j1, j2, j3, l;
    double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

    l = 2;
    if (n > 8) {
        M_cft1st(n, a);
        for(l = 8; ((l << 2) < n); l <<= 2)
            M_cftmdl(n, l, a);
    }
    if ((l << 2) == n) {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            j2 = j1 + l;
            j3 = j2 + l;
            x0i = -a[j + 1] - a[j1 + 1];
            x1i = -a[j + 1] + a[j1 + 1];
            x0r = a[j] + a[j1];
            x1r = a[j] - a[j1];
            x2i = a[j2 + 1] + a[j3 + 1];
            x3i = a[j2 + 1] - a[j3 + 1];
            x2r = a[j2] + a[j3];
            x3r = a[j2] - a[j3];
            a[j] = x0r + x2r;
            a[j + 1] = x0i - x2i;
            a[j2] = x0r - x2r;
            a[j2 + 1] = x0i + x2i;
            a[j1] = x1r - x3i;
            a[j1 + 1] = x1i - x3r;
            a[j3] = x1r + x3i;
            a[j3 + 1] = x1i + x3r;
        }
    } else {
        for (j = 0; j < l; j += 2) {
            j1 = j + l;
            x0i = -a[j + 1] + a[j1 + 1];
            x0r = a[j] - a[j1];
            a[j] += a[j1];
            a[j + 1] = -a[j + 1] - a[j1 + 1];
            a[j1] = x0r;
            a[j1 + 1] = x0i;
        }
    }
}


void M_rdft_forward(int n, double *a)
{
    if (n > 4) {
        M_bitrv2(n, a);
        M_cftfsub(n, a);
        M_rftfsub(n, a);
    } else
        M_cftfsub(n, a);
    double tmp = a[0] - a[1];
    a[0] += a[1];
    a[1] = tmp;
}


void M_rdft_inverse(int n, double *a)
{
    a[1] = 0.5 * (a[0] - a[1]);
    a[0] -= a[1];
    if (n > 4) {
        M_rftbsub(n, a);
        M_bitrv2(n, a);
        M_cftbsub(n, a);
    } else
        M_cftfsub(n, a);
}
