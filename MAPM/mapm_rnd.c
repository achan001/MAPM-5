#include "m_apm.h"
#include <time.h>

/*
        Used Knuth's The Art of Computer Programming, Volume 2 as
        the basis. Assuming the random number is X, compute
        (where all the math is performed on integers) :

        X = (a * X + c) MOD m

        From Knuth:

        'm' should be large, at least 2^30 : we use 1.0E+15

        'a' should be between .01m and .99m and not have a simple
        pattern. 'a' should not have any large factors in common
        with 'm' and (since 'm' is a power of 10) if 'a' MOD 200
        = 21 then all 'm' different possible values will be
        generated before 'X' starts to repeat.

        We use 'a' = 716805947629621.

        This is a prime number and also meets 'a' MOD 200 = 21.
        Commented out below are many potential multipliers that
        are all prime and meet 'a' MOD 200 = 21.

        There are few restrictions on 'c' except 'c' can have no
        factor in common with 'm', hence we set 'c' = 'a'.

        On the first call, the system time is used to initialize X.
*/

/*
 *  the following constants are all potential multipliers. they are
 *  all prime numbers that also meet the criteria of NUM mod 200 = 21.
 */

/*
439682071525421   439682071528421   439682071529221   439682071529821
439682071530421   439682071532021   439682071538821   439682071539421
439682071540021   439682071547021   439682071551221   439682071553821
439682071555421   439682071557221   439682071558021   439682071558621
439682071559821   439652381461621   439652381465221   439652381465621
439652381466421   439652381467421   439652381468621   439652381470021
439652381471221   439652381477021   439652381484221   439652381488421
439652381491021   439652381492021   439652381494021   439652381496821
617294387035621   617294387038621   617294387039221   617294387044421
617294387045221   617294387048621   617294387051621   617294387051821
617294387053621   617294387058421   617294387064221   617294387065621
617294387068621   617294387069221   617294387069821   617294387070421
617294387072021   617294387072621   617294387073821   617294387076821
649378126517621   649378126517821   649378126518221   649378126520821
649378126523821   649378126525621   649378126526621   649378126528421
649378126529621   649378126530821   649378126532221   649378126533221
649378126535221   649378126539421   649378126543621   649378126546021
649378126546421   649378126549421   649378126550821   649378126555021
649378126557421   649378126560221   649378126561621   649378126562021
649378126564621   649378126565821   672091582360421   672091582364221
672091582364621   672091582367021   672091582368421   672091582369021
672091582370821   672091582371421   672091582376821   672091582380821
716805243983221   716805243984821   716805947623621   716805947624621
716805947629021   716805947629621   716805947630621   716805947633621
716805947634221   716805947635021   716805947635621   716805947642221
*/

static  M_APM   M_rnd_aa;
static  M_APM   M_rnd_XX = NULL;

/****************************************************************************/
void M_free_all_rnd()
{
  if (M_rnd_XX == NULL) return;
  m_apm_free(M_rnd_aa);
  m_apm_free(M_rnd_XX);
  M_rnd_XX = NULL;
}
/****************************************************************************/
void M_get_rnd_seed(M_APM mm)
{
  m_apm_set_unsigned(mm, time(NULL));
}
/****************************************************************************/
void m_apm_set_random_seed(const char *ss)
{
  if (M_rnd_XX == NULL) {
    M_rnd_aa = m_apm_init();
    M_rnd_XX = m_apm_init();
    m_apm_set_string(M_rnd_aa, ".716805947629621");
  }
  m_apm_set_string(M_rnd_XX, ss);
}
/****************************************************************************/
/*
 *  compute X = (a * X + c) MOD m       where c = a
 *  m = 1e15, but 'scaled' so that m = 1
 */
void m_apm_get_random(M_APM mrnd)
{
  if (M_rnd_XX == NULL) {
    M_rnd_aa = m_apm_init();
    M_rnd_XX = m_apm_init();
    m_apm_set_string(M_rnd_aa, ".716805947629621");
    M_get_rnd_seed(M_rnd_XX);
  }
  m_apm_multiply(mrnd, M_rnd_XX, M_rnd_aa);
  m_apm_add(M_rnd_XX, mrnd, M_rnd_aa);
  m_apm_fraction(mrnd, M_rnd_XX);
  m_apm_copy(M_rnd_XX, mrnd);
  M_rnd_XX->m_apm_exponent += 15;
}
