#ifndef M__APM__INCLUDED
#define M__APM__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char UCHAR;

typedef struct {
    UCHAR *m_apm_data;
    int m_apm_id;
    int m_apm_refcount;     /* <- used only by C++ MAPM class */
    int m_apm_malloclength;
    int m_apm_datalength;
    int m_apm_exponent;
    int m_apm_sign;
} M_APM_struct;

typedef M_APM_struct *M_APM;

#define MAPM_LIB_SHORT_VERSION "5.1"    // 10/16/2019
#define MAPM_LIB_VERSION "MAPM Library Version 5.1 "\
        "Copyright (C) 1999-2007, Michael C. Ring"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"

#define  MAPM_MALLOC malloc
#define  MAPM_REALLOC realloc
#define  MAPM_FREE free

/*
 *  convienient predefined constants
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define M_APM_IDENT 0x6BCC9AE5
#define M_APM_RETURN 0
#define M_APM_FATAL 1

extern  int MM_lc_PI_digits;
extern  int MM_lc_log_digits;
extern  int M_stack_ptr;
extern  uint16_t *M_bcd_100;

extern  M_APM MM_Zero;
extern  M_APM MM_One;
extern  M_APM MM_Two;
extern  M_APM MM_Three;
extern  M_APM MM_Four;
extern  M_APM MM_Five;
extern  M_APM MM_Six;
extern  M_APM MM_Seven;
extern  M_APM MM_Eight;
extern  M_APM MM_Nine;
extern  M_APM MM_Ten;

extern  M_APM MM_0_5;
extern  M_APM MM_0_99995;
extern  M_APM MM_1024R;
extern  M_APM MM_E;
extern  M_APM MM_PI;
extern  M_APM MM_LOG_10;
extern  M_APM MM_LOG_10R;

extern  M_APM MM_lc_PI;
extern  M_APM MM_lc_HALF_PI;
extern  M_APM MM_lc_2_PI;
extern  M_APM MM_lc_LOG_10;
extern  M_APM MM_lc_LOG_10R;

// Debug macros
#define M_printf(...) fprintf(stderr, __VA_ARGS__)
#define M_puts(x, p) ({\
        char _buf[20 + (p == -1 ? (x)->m_apm_datalength : abs(p))];\
        m_apm_to_string(_buf, p, (x));\
        M_printf(#x " = %s\n", _buf);})

// function prototypes

M_APM   m_apm_init(void);
void    m_apm_free(M_APM);
void    m_apm_free_all_mem(void);
void    m_apm_trim_mem_usage(void);
char    *m_apm_lib_version(char *);
char    *m_apm_lib_short_version(char *);

void    m_apm_set_long(M_APM r, int n);
void    m_apm_set_unsigned(M_APM r, unsigned n);
void    m_apm_set_uint64_t(M_APM r, uint64_t n);
void    M_set_double(M_APM r, double x, int places);
void    M_set_string(M_APM r, const char *str, char **end);

#define m_apm_set_double(r, x)    M_set_double(r, x, 16)
#define m_apm_set_string(r, str)  M_set_string(r, str, NULL)

double  m_apm_to_double(M_APM);
void    m_apm_to_string(char *, int, M_APM);
void    m_apm_to_fixpt_string(char *, int, M_APM);
void    m_apm_to_fixpt_stringex(char *, int, M_APM, char, char, int);
char    *m_apm_to_fixpt_stringexp(int, M_APM, char, char, int);
void    m_apm_to_integer_string(char *, M_APM);

void    m_apm_absolute_value(M_APM, M_APM);
void    m_apm_negate(M_APM, M_APM);
void    m_apm_copy(M_APM, M_APM);
void    m_apm_iround_fixpt(M_APM, int);

void    m_apm_iround(M_APM r, int);
void    m_apm_round(M_APM, int, M_APM);
void    m_apm_ichop(M_APM r, int places);

int     m_apm_compare(M_APM, M_APM);
int     m_apm_compare_absolute(M_APM ltmp, M_APM rtmp);
int     m_apm_sign(M_APM);
int     m_apm_exponent(M_APM);
int     m_apm_significant_digits(M_APM);
int     m_apm_is_integer(M_APM);
int     m_apm_is_odd(M_APM);
#define m_apm_is_even(x)  (!m_apm_is_odd(x))

void    m_apm_gcd(M_APM, M_APM, M_APM);
void    m_apm_lcm(M_APM, M_APM, M_APM);

void    m_apm_add(M_APM, M_APM, M_APM);
void    m_apm_subtract(M_APM, M_APM, M_APM);
void    m_apm_multiply(M_APM, M_APM, M_APM);

#ifdef NO_MAPM_SQR
#define m_apm_square(r, x)   m_apm_multiply(r, x, x)
#else
void    m_apm_square(M_APM, M_APM);
#endif

void    m_apm_divide(M_APM, int, M_APM, M_APM);
void    m_apm_integer_divide(M_APM, M_APM, M_APM);
void    m_apm_integer_div_rem(M_APM, M_APM, M_APM, M_APM);
void    m_apm_reciprocal(M_APM, int, M_APM);

void    m_apm_factorial(M_APM, M_APM);
void    m_apm_floor(M_APM, M_APM);
void    m_apm_ceil(M_APM, M_APM);
void    m_apm_integer(M_APM r, M_APM x);
void    m_apm_away(M_APM r, M_APM x);
void    m_apm_fraction(M_APM r, M_APM x);
void    m_apm_get_random(M_APM);
void    m_apm_set_random_seed(const char *);

void    m_apm_sqrt(M_APM, int, M_APM);
void    m_apm_cbrt(M_APM, int, M_APM);
void    m_apm_log(M_APM, int, M_APM);
void    m_apm_log10(M_APM, int, M_APM);
void    m_apm_exp(M_APM, int, M_APM);
void    m_apm_pow(M_APM, int, M_APM, M_APM);
void    m_apm_integer_pow(M_APM, int, M_APM, int);
void    m_apm_integer_pow_nr(M_APM, M_APM, unsigned);
void    m_apm_ishift(M_APM, int);
void    m_apm_powmod(M_APM, M_APM, M_APM, M_APM);

void    m_apm_sin_cos(M_APM, M_APM, int, M_APM);
void    m_apm_sin(M_APM, int, M_APM);
void    m_apm_cos(M_APM, int, M_APM);
void    m_apm_tan(M_APM, int, M_APM);
void    m_apm_arcsin(M_APM, int, M_APM);
void    m_apm_arccos(M_APM, int, M_APM);
void    m_apm_arctan(M_APM, int, M_APM);
void    m_apm_arctan2(M_APM, int, M_APM, M_APM);

void    m_apm_sinh(M_APM, int, M_APM);
void    m_apm_cosh(M_APM, int, M_APM);
void    m_apm_tanh(M_APM, int, M_APM);
void    m_apm_arcsinh(M_APM, int, M_APM);
void    m_apm_arccosh(M_APM, int, M_APM);
void    m_apm_arctanh(M_APM, int, M_APM);

M_APM   M_get_stack_var(void);
#define M_restore_stack(count)  (void) (M_stack_ptr -= (count))

void    M_free_all_fft(void);
void    M_fast_multiply(M_APM, M_APM, M_APM);
void    M_apm_divide(M_APM, int, M_APM, M_APM);

void    M_init_all_cnst(void);
void    M_init_util_data(void);
void    M_free_all_add(void);
void    M_free_all_div(void);
void    M_free_all_rnd(void);
void    M_free_all_cnst(void);
void    M_free_all_fmul(void);
void    M_free_all_stck(void);
void    M_free_all_util(void);

void    M_raw_sin(M_APM, int, M_APM);   // note: ~places do sinh(x)
void    M_625x_sin(M_APM, int, M_APM);
void    M_625x_cos(M_APM, int, M_APM);
void    M_sin_cos(M_APM sinv, M_APM cosv, int places, M_APM aa);
void    M_sin_to_cos(M_APM r , int places, M_APM x, int flip);
void    M_limit_angle_to_pi(M_APM, int, M_APM);
void    M_arcsin_near_0(M_APM, int, M_APM);
void    M_arccos_near_0(M_APM, int, M_APM);
void    M_arctan_near_0(M_APM, int, M_APM);
void    M_arctan_large_input(M_APM, int, M_APM);
void    M_check_PI_places(int);
void    M_calculate_PI_AGM(M_APM, int);

void    M_integer_pos_pow(M_APM r, int dplaces, M_APM x, unsigned n);
void    M_apm_exp(M_APM, int, M_APM);
void    M_apm_log(M_APM, int, M_APM);
void    M_log_basic_iteration(M_APM, int, M_APM);
void    M_check_log_places(int);

void    M_realloc(M_APM r, unsigned n);
void    M_apm_normalize(M_APM);
void    M_apm_scale(M_APM, int);
void    M_apm_pad(M_APM, int, int);
void    M_set_to_zero(M_APM);
void    M_set_samesign(M_APM r, unsigned n);
void    M_apm_error(int, const char *);
char*   M_to_data_str(char *s, M_APM r, int len, int digits);
int     M_fixpt_bytes(M_APM x, int places, int cnt);
int     M_ilog10(unsigned n);

void    M_add_digit(M_APM r, int digit);
void    M_mul_digit(M_APM r, M_APM x, int digit);
#define M_mul_digit_exp(r, x, digit, dexp) ({\
        M_mul_digit(r, x, digit);\
        if (r->m_apm_sign) r->m_apm_exponent += dexp;})

void    M_add_samesign(M_APM r, M_APM a, M_APM b);
void    M_sub_samesign(M_APM r, M_APM a, M_APM b);
void    M_factorial(M_APM r, int n);
void    M_reciprocal(M_APM r, int places, M_APM N);
void    M_sqrt_flip(M_APM r, int places, M_APM N, int flip);
void    M_mul_edge(M_APM r, int a, int b, int n);
void    M_exp_pair(M_APM t1, M_APM t2, int places, M_APM x);

#define m_apm_asin m_apm_arcsin
#define m_apm_acos m_apm_arccos
#define m_apm_atan m_apm_arctan
#define m_apm_atan2 m_apm_arctan2
#define m_apm_asinh m_apm_arcsinh
#define m_apm_acosh m_apm_arccosh
#define m_apm_atanh m_apm_arctanh

#ifdef __cplusplus
}      /* End extern "C" bracket */
#endif

#ifdef __cplusplus  /* Hides the class below from C compilers */
/*
    This class lets you use M_APM's a bit more intuitively with
    C++'s operator and function overloading, constructors, etc.
    Added 3/24/2000 by Orion Sky Lawlor, olawlor@acm.org
*/

extern
#ifdef __cplusplus
"C"
#endif
int MM_cpp_min_precision;
#define m_apm_cpp_precision(digits) (MM_cpp_min_precision = (digits))

/*
The M_APM structure here is implemented as a reference-
counted, copy-on-write data structure-- this makes copies
very fast, but that's why it's so ugly.  A MAPM object is
basically just a wrapper around a (possibly shared)
M_APM_struct myVal.
*/

class MAPM {
protected:

  M_APM myVal;  /* My M_APM structure */
  void create(void) {myVal=makeNew();}
  void destroy(void) {unref(myVal);myVal=NULL;}
  void copyFrom(M_APM Nval) {
    M_APM oldVal=myVal;
    myVal=Nval;
    ref(myVal);
    unref(oldVal);
  }
  static M_APM makeNew(void) {
    M_APM val=m_apm_init();
    return val; /* initialize 1 by 'm_apm_init' */
  }
  static void ref(M_APM val) {
    val->m_apm_refcount++;
  }
  static void unref(M_APM val) {
    val->m_apm_refcount--;
    if (val->m_apm_refcount==0)
      m_apm_free(val);
  }

  M_APM val(void) {   // return a private (mutable) copy
    if (myVal->m_apm_refcount==1) return myVal;
    /* Return my private myVal, otherwise ... */
    /* our copy of myVal is shared -- need new private copy */
    M_APM oldVal=myVal;
    myVal=makeNew();
    m_apm_copy(myVal,oldVal);
    unref(oldVal);
    return myVal;
  }

  /*BAD: C M_APM routines doesn't use "const" where they should--
    hence we have to cast to a non-const type here (FIX THIS!).
    (in due time.... MCR)
  */
  /* This is the default number of digits to use for
     1-ary functions like sin, cos, tan, etc.
     It's the larger of my digits and cpp_min_precision.
      */
  int myDigits(void) const {
    int maxd = m_apm_significant_digits(cval());
    if (maxd<MM_cpp_min_precision) maxd=MM_cpp_min_precision;
    return maxd;
  }
  /* This is the default number of digits to use for
     2-ary functions like divide, atan2, etc.
     It's the larger of my digits, his digits, and cpp_min_precision.
      */
  int digits(const MAPM &otherVal) const {
    int maxd=myDigits();
    int his=m_apm_significant_digits(otherVal.cval());
    if (maxd<his) maxd=his;
    return maxd;
  }

public:
  M_APM cval(void) const { return (M_APM)myVal; }
  MAPM(void) {create();}    /* Constructors: */
  MAPM(const MAPM &m) {myVal=(M_APM)m.cval();ref(myVal);}
  MAPM(M_APM m)       {myVal=(M_APM)m;ref(myVal);}
  MAPM(const char *s) {create();m_apm_set_string(val(),(char *)s);}
  MAPM(int l)         {create();m_apm_set_long(val(),l);}
  MAPM(long l)        {create();m_apm_set_long(val(),l);}
  MAPM(unsigned l)    {create();m_apm_set_unsigned(val(),l);}
  MAPM(uint64_t l)    {create();m_apm_set_uint64_t(val(),l);}
  MAPM(int64_t l)     {create(); int64_t n = l<0 ? -l : l;
    m_apm_set_uint64_t(val(), n); if (l<0) val()->m_apm_sign = -1;}

  /* NOTE: m_apm_set_double now return 17 sig. digits */
  /* Exact conversion, use MAPM(x, -1) */

  MAPM(double d)             {create();m_apm_set_double(val(),d);}
  MAPM(double d, int places) {create();M_set_double(val(),d,places);}

  ~MAPM() {destroy();}      /* Destructor */

  /* Extracting string descriptions: */
  void toString(char *dest,int decimalPlaces) const
    {m_apm_to_string(dest,decimalPlaces,cval());}
  void toFixPtString(char *dest,int decimalPlaces) const
    {m_apm_to_fixpt_string(dest,decimalPlaces,cval());}
  void toFixPtStringEx(char *dest,int dp,char a,char b,int c) const
    {m_apm_to_fixpt_stringex(dest,dp,cval(),a,b,c);}
  char *toFixPtStringExp(int dp,char a,char b,int c) const
    {return(m_apm_to_fixpt_stringexp(dp,cval(),a,b,c));}
  void toIntegerString(char *dest) const
    {m_apm_to_integer_string(dest,cval());}

  /* Basic operators: */
  MAPM &operator=(const MAPM &m) /* Assigment operator */
    {copyFrom((M_APM)m.cval());return *this;}
  MAPM &operator=(const char *s) /* Assigment operator */
    {m_apm_set_string(val(),(char *)s);return *this;}
  MAPM &operator=(double d) /* Assigment operator */
    {m_apm_set_double(val(),d);return *this;}
  MAPM &operator=(int l) /* Assigment operator */
    {m_apm_set_long(val(),l);return *this;}
  MAPM &operator=(long l) /* Assigment operator */
    {m_apm_set_long(val(),l);return *this;}
  MAPM operator++() /* Prefix increment operator */
    {return *this = *this+MM_One;}
  MAPM operator--() /* Prefix decrement operator */
    {return *this = *this-MM_One;}
  const MAPM operator++(int)
    {MAPM old = *this; ++(*this); return old;}
  const MAPM operator--(int)
    {MAPM old = *this; --(*this); return old;}

  /* Comparison operators */
  int operator==(const MAPM &m) const
    {return m_apm_compare(cval(),m.cval())==0;}
  int operator!=(const MAPM &m) const
    {return m_apm_compare(cval(),m.cval())!=0;}
  int operator<(const MAPM &m) const
    {return m_apm_compare(cval(),m.cval())<0;}
  int operator<=(const MAPM &m) const
    {return m_apm_compare(cval(),m.cval())<=0;}
  int operator>(const MAPM &m) const
    {return m_apm_compare(cval(),m.cval())>0;}
  int operator>=(const MAPM &m) const
    {return m_apm_compare(cval(),m.cval())>=0;}

  /* Basic arithmetic operators */
  friend MAPM operator+(const MAPM &a,const MAPM &b)
    {MAPM ret;m_apm_add(ret.val(),a.cval(),b.cval());return ret;}
  friend MAPM operator-(const MAPM &a,const MAPM &b)
    {MAPM ret;m_apm_subtract(ret.val(),a.cval(),b.cval());return ret;}
  friend MAPM operator*(const MAPM &a,const MAPM &b)
    {MAPM ret;m_apm_multiply(ret.val(),a.cval(),b.cval());return ret;}
  friend MAPM operator%(const MAPM &a,const MAPM &b)
    {MAPM quot,ret;
    m_apm_integer_div_rem(quot.val(),ret.val(),a.cval(),b.cval());
    return ret;}

  /* Default division keeps larger of cpp_min_precision, numerator */
  /* digits of precision, or denominator digits of precision. */
  friend MAPM operator/(const MAPM &a,const MAPM &b)
    {return a.divide(b,a.digits(b));}

  MAPM divide(const MAPM &m) const {return divide(m,digits(m));}
  MAPM divide(const MAPM &m,int places) const
    {MAPM ret;m_apm_divide(ret.val(),places,cval(),m.cval());return ret;}

  /* Assignment arithmetic operators */
  MAPM &operator+=(const MAPM &m) {*this = *this+m;return *this;}
  MAPM &operator-=(const MAPM &m) {*this = *this-m;return *this;}
  MAPM &operator*=(const MAPM &m) {*this = *this*m;return *this;}
  MAPM &operator/=(const MAPM &m) {*this = *this/m;return *this;}
  MAPM &operator%=(const MAPM &m) {*this = *this%m;return *this;}

  /* Extracting/setting simple information: */
  int sign(void) const {return m_apm_sign(cval());}
  int exponent(void) const {return m_apm_exponent(cval());}
  int significant_digits(void) const
    {return m_apm_significant_digits(cval());}
  int is_integer(void) const {return m_apm_is_integer(cval());}
  int is_even(void) const {return m_apm_is_even(cval());}
  int is_odd(void) const {return m_apm_is_odd(cval());}
  double toDouble(void) const {return m_apm_to_double(cval());}

  /* Functions: */
  MAPM sqr(void) const {MAPM ret;
    m_apm_square(ret.val(),cval());return ret;}  
  MAPM abs(void) const {MAPM ret;
    m_apm_absolute_value(ret.val(),cval());return ret;}
  MAPM neg(void) const {MAPM ret;
    m_apm_negate(ret.val(),cval());return ret;}
  MAPM round(int places) const {MAPM ret;
    m_apm_round(ret.val(),places,cval());return ret;}
  MAPM operator-(void) const {return neg();}

/* I got tired of typing the various declarations for a simple
   1-ary real-to-real function on MAPM's; hence this define:
   The digits-free versions return my digits of precision or
   cpp_min_precision, whichever is bigger.
*/

#define MAPM_1aryFunc(func) \
  MAPM func(int places) const\
    {MAPM ret;m_apm_##func(ret.val(),places,cval());return ret;}\
  MAPM func(void) const {return func(myDigits());}

  MAPM_1aryFunc(sqrt)
  MAPM_1aryFunc(cbrt)
  MAPM_1aryFunc(log)
  MAPM_1aryFunc(exp)
  MAPM_1aryFunc(log10)
  MAPM_1aryFunc(sin)
  MAPM_1aryFunc(asin)
  MAPM_1aryFunc(cos)
  MAPM_1aryFunc(acos)
  MAPM_1aryFunc(tan)
  MAPM_1aryFunc(atan)
  MAPM_1aryFunc(sinh)
  MAPM_1aryFunc(asinh)
  MAPM_1aryFunc(cosh)
  MAPM_1aryFunc(acosh)
  MAPM_1aryFunc(tanh)
  MAPM_1aryFunc(atanh)
#undef MAPM_1aryFunc

  void sincos(MAPM &sinR,MAPM &cosR,int places)
    {m_apm_sin_cos(sinR.val(),cosR.val(),places,cval());}
  void sincos(MAPM &sinR,MAPM &cosR)
    {sincos(sinR,cosR,myDigits());}
  MAPM pow(const MAPM &m,int places) const
    {MAPM ret;m_apm_pow(ret.val(),places,cval(),m.cval());return ret;}
  MAPM pow(const MAPM &m) const {return pow(m,digits(m));}
  MAPM atan2(const MAPM &x,int places) const
    {MAPM ret;m_apm_arctan2(ret.val(),places,cval(),x.cval());return ret;}
  MAPM atan2(const MAPM &x) const {return atan2(x,digits(x));}

  MAPM gcd(const MAPM &m) const {MAPM ret;
    m_apm_gcd(ret.val(),cval(),m.cval());return ret;}
  MAPM lcm(const MAPM &m) const {MAPM ret;
    m_apm_lcm(ret.val(),cval(),m.cval());return ret;}

  static MAPM random(void) {MAPM ret;
    m_apm_get_random(ret.val());return ret;}

  MAPM floor(void) const {MAPM ret;
    m_apm_floor(ret.val(),cval());return ret;}
  MAPM ceil(void) const {MAPM ret;
    m_apm_ceil(ret.val(),cval());return ret;}

  /* Functions defined only on integers: */
  MAPM factorial(void) const
    {MAPM ret;m_apm_factorial(ret.val(),cval());return ret;}
  MAPM ipow_nr(int p) const
    {MAPM ret;m_apm_integer_pow_nr(ret.val(),cval(),p);return ret;}
  MAPM ipow(int p,int places) const {MAPM ret;
    m_apm_integer_pow(ret.val(),places,cval(),p);return ret;}
  MAPM ipow(int p) const {return ipow(p,myDigits());}
  MAPM integer_divide(const MAPM &denom) const {MAPM ret;
    m_apm_integer_divide(ret.val(),cval(),denom.cval());return ret;}
  void integer_div_rem(const MAPM &denom,MAPM &quot,MAPM &rem) const
    {m_apm_integer_div_rem(quot.val(),rem.val(),cval(),denom.cval());}
  MAPM div(const MAPM &denom) const {return integer_divide(denom);}
  MAPM rem(const MAPM &denom) const
    {MAPM ret,ignored;integer_div_rem(denom,ignored,ret);return ret;}
};

/* math.h-style functions: */

inline MAPM fabs(const MAPM &m) {return m.abs();}
inline MAPM factorial(const MAPM &m) {return m.factorial();}
inline MAPM floor(const MAPM &m) {return m.floor();}
inline MAPM ceil(const MAPM &m) {return m.ceil();}
inline MAPM get_random(void) {return MAPM::random();}

/* I got tired of typing the various declarations for a simple
   1-ary real-to-real function on MAPM's; hence this define:
*/
#define MAPM_1aryFunc(func) \
  inline MAPM func(const MAPM &m) {return m.func();} \
  inline MAPM func(const MAPM &m,int places) {return m.func(places);}
  MAPM_1aryFunc(sqrt)
  MAPM_1aryFunc(cbrt)
  MAPM_1aryFunc(log)
  MAPM_1aryFunc(exp)
  MAPM_1aryFunc(log10)
  MAPM_1aryFunc(sin)
  MAPM_1aryFunc(asin)
  MAPM_1aryFunc(cos)
  MAPM_1aryFunc(acos)
  MAPM_1aryFunc(tan)
  MAPM_1aryFunc(atan)
  MAPM_1aryFunc(sinh)
  MAPM_1aryFunc(asinh)
  MAPM_1aryFunc(cosh)
  MAPM_1aryFunc(acosh)
  MAPM_1aryFunc(tanh)
  MAPM_1aryFunc(atanh)
#undef MAPM_1aryFunc

inline MAPM pow(const MAPM &x,const MAPM &y)   {return x.pow(y);}
inline MAPM pow(const MAPM &x,const MAPM &y,int places)
  {return x.pow(y,places);}

inline MAPM atan2(const MAPM &y,const MAPM &x) {return y.atan2(x);}
inline MAPM atan2(const MAPM &y,const MAPM &x,int places)
  {return y.atan2(x,places);}

inline MAPM gcd(const MAPM &u,const MAPM &v)   {return u.gcd(v);}
inline MAPM lcm(const MAPM &u,const MAPM &v)   {return u.lcm(v);}
#endif
#endif
