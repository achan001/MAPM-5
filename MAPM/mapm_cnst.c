#include "m_apm.h"

int MM_lc_PI_digits = 0;
int MM_lc_log_digits = 0;
int MM_cpp_min_precision = 30;  /* only used in C++ wrapper */

M_APM MM_Zero    = NULL;
M_APM MM_One     = NULL;
M_APM MM_Two     = NULL;
M_APM MM_Three   = NULL;
M_APM MM_Four    = NULL;
M_APM MM_Five    = NULL;
M_APM MM_Six     = NULL;
M_APM MM_Seven   = NULL;
M_APM MM_Eight   = NULL;
M_APM MM_Nine    = NULL;
M_APM MM_Ten     = NULL;
M_APM MM_0_5     = NULL;
M_APM MM_0_99995 = NULL;
M_APM MM_1024R   = NULL;
M_APM MM_E       = NULL;
M_APM MM_PI      = NULL;
M_APM MM_LOG_10  = NULL;
M_APM MM_LOG_10R = NULL;

M_APM MM_lc_PI      = NULL;
M_APM MM_lc_HALF_PI = NULL;
M_APM MM_lc_2_PI    = NULL;
M_APM MM_lc_LOG_10  = NULL;
M_APM MM_lc_LOG_10R = NULL;

#define VALID_DECIMAL_PLACES 120

static char MM_cnst_PI[] =
"3.14159265358979323846264338327950288419716939937510582097494"
"4592307816406286208998628034825342117067982148086513282306647094";

static char MM_cnst_E[] =
"2.71828182845904523536028747135266249775724709369995957496696"
"7627724076630353547594571382178525166427427466391932003059921817";

static char MM_cnst_log_10[] =
"2.30258509299404568401799145468436420760110148862877297603332"
"7900967572609677352480235997205089598298341967784042286248633410";

static char MM_cnst_log_10R[] =
".434294481903251827651128918916605082294397005803666566114453"
"7831658646492088707747292249493384317483187061067447663037336417";

/****************************************************************************/
char *m_apm_lib_version(char *v)
{
  return strcpy(v, MAPM_LIB_VERSION);
}
/****************************************************************************/
char *m_apm_lib_short_version(char *v)
{
  return strcpy(v, MAPM_LIB_SHORT_VERSION);
}
/****************************************************************************/
void M_free_all_cnst()
{
  if (MM_lc_PI_digits == 0) return;
  MM_lc_PI_digits = 0;

  m_apm_free(MM_Zero);
  m_apm_free(MM_One);
  m_apm_free(MM_Two);
  m_apm_free(MM_Three);
  m_apm_free(MM_Four);
  m_apm_free(MM_Five);
  m_apm_free(MM_Six);
  m_apm_free(MM_Seven);
  m_apm_free(MM_Eight);
  m_apm_free(MM_Nine);
  m_apm_free(MM_Ten);
  m_apm_free(MM_0_5);
  m_apm_free(MM_0_99995);
  m_apm_free(MM_1024R);
  m_apm_free(MM_E);
  m_apm_free(MM_PI);
  m_apm_free(MM_lc_PI);
  m_apm_free(MM_lc_HALF_PI);
  m_apm_free(MM_lc_2_PI);
  m_apm_free(MM_lc_LOG_10);
  m_apm_free(MM_lc_LOG_10R);
  m_apm_free(MM_LOG_10R);
  m_apm_free(MM_LOG_10);
}
/****************************************************************************/
void M_init_all_cnst()
{
  if (MM_lc_PI_digits) return;
  MM_lc_PI_digits  = VALID_DECIMAL_PLACES;
  MM_lc_log_digits = VALID_DECIMAL_PLACES;

  MM_Zero          = m_apm_init();
  MM_One           = m_apm_init();
  MM_Two           = m_apm_init();
  MM_Three         = m_apm_init();
  MM_Four          = m_apm_init();
  MM_Five          = m_apm_init();
  MM_Six           = m_apm_init();
  MM_Seven         = m_apm_init();
  MM_Eight         = m_apm_init();
  MM_Nine          = m_apm_init();
  MM_Ten           = m_apm_init();
  MM_0_5           = m_apm_init();
  MM_0_99995       = m_apm_init();
  MM_1024R         = m_apm_init();
  MM_E             = m_apm_init();
  MM_PI            = m_apm_init();
  MM_lc_PI         = m_apm_init();
  MM_lc_HALF_PI    = m_apm_init();
  MM_lc_2_PI       = m_apm_init();
  MM_lc_LOG_10     = m_apm_init();
  MM_lc_LOG_10R    = m_apm_init();
  MM_LOG_10R = m_apm_init();
  MM_LOG_10 = m_apm_init();

  m_apm_set_unsigned(MM_One, 1);
  m_apm_set_unsigned(MM_Two, 2);
  m_apm_set_unsigned(MM_Three, 3);
  m_apm_set_unsigned(MM_Four, 4);
  m_apm_set_unsigned(MM_Five, 5);
  m_apm_set_unsigned(MM_Six, 6);
  m_apm_set_unsigned(MM_Seven, 7);
  m_apm_set_unsigned(MM_Eight, 8);
  m_apm_set_unsigned(MM_Nine, 9);
  m_apm_set_unsigned(MM_Ten, 10);

  m_apm_set_string(MM_0_5,  ".5");
  m_apm_set_string(MM_0_99995, ".99995");
  m_apm_set_string(MM_1024R, ".9765625e-3");

  m_apm_set_string(MM_LOG_10, MM_cnst_log_10);
  m_apm_set_string(MM_LOG_10R, MM_cnst_log_10R);
  m_apm_set_string(MM_E, MM_cnst_E);
  m_apm_set_string(MM_PI, MM_cnst_PI);

  m_apm_copy(MM_lc_LOG_10, MM_LOG_10);
  m_apm_copy(MM_lc_LOG_10R, MM_LOG_10R);
  m_apm_copy(MM_lc_PI, MM_PI);
  m_apm_multiply(MM_lc_HALF_PI, MM_PI, MM_0_5);
  m_apm_multiply(MM_lc_2_PI,    MM_PI, MM_Two);
}
