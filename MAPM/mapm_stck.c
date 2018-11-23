#include "m_apm.h"

int M_stack_ptr = -1;   // global variable

static int M_last_init  = -1;
static int M_stack_size = 0;

static char M_stack_err_msg[] = "M_grow_stack, Out of memory";

static M_APM *M_stack_array;

/****************************************************************************/
void M_free_all_stck()
{
  if (M_last_init == -1) return;

  for (int k=0; k <= M_last_init; k++)
    m_apm_free(M_stack_array[k]);

  M_stack_ptr  = -1;
  M_last_init  = -1;
  M_stack_size = 0;
  MAPM_FREE(M_stack_array);
}
/****************************************************************************/
static void M_grow_stack()
{
  if (M_stack_size == 0) {
    M_stack_size = 18;
    void *vp = MAPM_MALLOC(M_stack_size * sizeof(M_APM));
    if (!vp) M_apm_error(M_APM_FATAL, M_stack_err_msg);
    M_stack_array = (M_APM *)vp;
  }

  if ((M_last_init += 4) >= M_stack_size) {
    M_stack_size += 12;
    void *vp = MAPM_REALLOC(M_stack_array, M_stack_size * sizeof(M_APM));
    if (!vp) M_apm_error(M_APM_FATAL, M_stack_err_msg);
    M_stack_array = (M_APM *) vp;
  }

  M_stack_array[M_stack_ptr]     = m_apm_init();
  M_stack_array[M_stack_ptr + 1] = m_apm_init();
  M_stack_array[M_stack_ptr + 2] = m_apm_init();
  M_stack_array[M_stack_ptr + 3] = m_apm_init();
}

/****************************************************************************/

M_APM M_get_stack_var()
{
  if (M_stack_ptr++ >= M_last_init) M_grow_stack();
  return M_stack_array[M_stack_ptr];
}

