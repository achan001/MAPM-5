#include <fcntl.h>
#include "m_apm.h"

#define MM    0x00    // ( MAPM, MAPM )
#define MiM   0x10    // ( MAPM, int, MAPM )
#define MMM   0x01    // ( MAPM, MAPM, MAPM )
#define Mi2   0x11    // ( MAPM, int, MAPM, MAPM )
#define MMA   0x02    // ( MAPM, MAPM )
#define MiA   0x12    // ( MAPM, int )
#define MEM   37      // memories: 0-9,a-z,{
#define TOP   62      // working stack
#define ASIZE(a)      (sizeof(a) / sizeof(a[0]))

char buf[8192];
char lookup[256];
int _CRT_glob = 0;    // NO globbing
int places    = 35;   // ~ binary128
M_APM stack[MEM + 1 + TOP];
M_APM *base = stack + MEM, *top = stack + MEM;

static void calc_pi(M_APM r, int places);
static void calc_rem(M_APM r, M_APM x, M_APM y);
static void calc_mod(M_APM r, M_APM x, M_APM y);

struct opstr {
  UCHAR name[8];
  int  type;
  void (*func)();
};
struct opstr cmds[] = {                 /*** MUST BE SORTED ***/
  {"!",    MM, m_apm_factorial     },
  {"%",    MMM,calc_rem            },   // rem = @2 @2 // x -
  {"%%",   MMM,calc_mod            },   // mod = % @ + @ %
  {"*",    MMM,m_apm_multiply      },   // agm = sb + @- rb x sqrt $ 2/ ?s
  {"+",    MMM,m_apm_add           },
  {"-",    MMM,m_apm_subtract      },
  {"/",    Mi2,m_apm_divide        },
  {"//",   MMM,m_apm_integer_divide},
  {"\\",   MiM,m_apm_reciprocal    },
  {"abs",  MM, m_apm_absolute_value},
  {"acos", MiM,m_apm_acos          },
  {"acosh",MiM,m_apm_acosh         },
  {"asin", MiM,m_apm_asin          },
  {"asinh",MiM,m_apm_asinh         },
  {"atan", MiM,m_apm_atan          },
  {"atan2",Mi2,m_apm_atan2         },
  {"atanh",MiM,m_apm_atanh         },
  {"away", MM, m_apm_away          },
  {"cbrt", MiM,m_apm_cbrt          },
  {"ceil", MM, m_apm_ceil          },
  {"cos",  MiM,m_apm_cos           },
  {"cosh", MiM,m_apm_cosh          },
  {"d",    MMA,m_apm_copy          },
  {"exp",  MiM,m_apm_exp           },
  {"floor",MM, m_apm_floor         },
  {"frac", MM, m_apm_fraction      },
  {"gcd",  MMM,m_apm_gcd           },
  {"int",  MM, m_apm_integer       },
  {"lcm",  MMM,m_apm_lcm           },
  {"log",  MiM,m_apm_log           },
  {"log10",MiM,m_apm_log10         },
  {"n",    MM, m_apm_negate        },
  {"p",    Mi2,m_apm_pow           },
  {"pi",   MiA,calc_pi             },
  {"sin",  MiM,m_apm_sin           },
  {"sinh", MiM,m_apm_sinh          },
  {"sqr",  MM, m_apm_square        },
  {"sqrt", MiM,m_apm_sqrt          },
  {"tan",  MiM,m_apm_tan           },
  {"tanh", MiM,m_apm_tanh          },
  {"x",    MMM,m_apm_multiply      },
  {"\xff", 0  , NULL}};                 /*** MUST BE LAST ***/

char usage[] =
"rpn [numbers/cmds ...]    default s = 36 decimal digits\n"
"        + : add                 sin : sin\n"
"        - : subtract            cos : cos\n"
"        x : multiply (*)        tan : tan\n"
"        / : divide             asin : arc-sin\n"
"       // : integer divide     acos : arc-cos\n"
"     %[%] : remainder/modulo   atan : arc-tan\n"
"        \\ : flip (1/x)        atan2 : arc-tan2(y, x)\n"
"        n : negate (-x)        sinh : sinh\n"
"        d : dup (HP-Enter)     cosh : cosh\n"
"   $[s|r] : swap, opt=sort     tanh : tanh\n"
"      sqr : square            asinh : arc-sinh\n"
"     sqrt : square root       acosh : arc-cosh\n"
"     cbrt : cube root         atanh : arc-tanh\n"
"        p : y ^ x            ?[?|s] : show stack(s), no rounding\n"
"      exp : e ^ x                ?# : show x rounded to # digits\n"
"      log : log(e)               ?m : show x in mathematica form\n"
"    log10 : log10                ?l : show x len of digits\n"
"        ! : factorial           ?i# : show rounded integer, #=add comma\n"
"      gcd : gcd                  =# : set s=#, x rounded to s digits\n"
"      lcm : lcm                  f# : set x to # fixpt precision\n"
"    floor : floor                g# : set x to # digit IEEE double\n"
"     ceil : ceiling   [# cmds ]file : read file,  #=0-9,a-z\n"
"      int : integer              k# : kill stack  #=0-9,a-z\n"
"     frac : fraction        m[+/-]# : m+ (default) or m- x\n"
"      abs : absolute value       r# : read memory #=0-9,a-z\n"
"     away : away from zero       s# : save memory #=0-9,a-z\n"
"       pi : 3.14159 ...          @# : read stack  1=top, -1=bottom";
/*******************************************************************/
int puts(const char *s)         // puts() has bug in binary mode
{
    while (*s) putchar(*s++);   // try rpn 1e99999 1+ ?i k
    return putchar('\n');       // if it ends in 0's, puts is bad
}
/*******************************************************************/
static void calc_rem(M_APM r, M_APM x, M_APM y)
{
  M_APM q = M_get_stack_var();
  m_apm_integer_div_rem(q, r, x, y);
  M_restore_stack(1);
}
/*******************************************************************/
static void calc_mod(M_APM r, M_APM x, M_APM y)
{
  calc_rem(r, x, y);
  if (r->m_apm_sign * y->m_apm_sign == -1) {
    M_APM t = M_get_stack_var();
    m_apm_copy(t, r);
    m_apm_add(r, t, y);     // match sign of y
    M_restore_stack(1);
  }
}
/*******************************************************************/
static void calc_pi(M_APM r, int p)
{
  M_check_PI_places(p);
  m_apm_round(r, p, MM_lc_PI);
}
/*******************************************************************/
static void show_length(M_APM x)
{
  int len = x->m_apm_datalength;
  int sgn = "-?+"[x->m_apm_sign + 1];
  printf("%c0.(%d digits) 10^%d\n", sgn, len, x->m_apm_exponent);
}
/*******************************************************************/
static void show_rint(M_APM x, char *str)
{
  int count = strtol(str, NULL,10);
  char *buf = m_apm_to_fixpt_stringexp(0, x, 0, ',', count);
  puts(buf);
  free(buf);
}
/*******************************************************************/
static void show_mathematica(M_APM x)
{
  if (x->m_apm_sign == 0) {puts("+0"); return;}
  int n = x->m_apm_datalength;
  char *buf = malloc(n + 20), *s = buf;
  if (buf == NULL) {puts("rpn: out of memory"); exit(1);}

  *s++ = x->m_apm_sign < 0 ? '-' : '+'; // explicit sign
  s = M_to_data_str(s, x, n, 0);
  if ((n = x->m_apm_exponent - n)) {    // assume no overflow
    *s++ = n>0 ? '*' : (n=-n, '/');     // Wolfram-Alpha ready
    *s++ = '1'; *s++ = '0'; *s++ = '^'; // avoid negative nexp
    itoa(n, s, 10);
  }
  puts(buf);
  free(buf);
}
/*******************************************************************/
static void show_number(M_APM x)
{
  static void (*f[])() = {m_apm_to_string, m_apm_to_fixpt_string};
  char *buf = malloc(x->m_apm_datalength + 16);
  if (buf == NULL) {puts("rpn: out of memory"); exit(1);}
  f[INRANGE(m_apm_exponent(x), -4, 6)](buf, -1, x);
  puts(buf);
  free(buf);
}
/*******************************************************************/
static int mem_idx(int c)   // return 0 to 36
{                           // note: '{' = 'z' + 1
  if (INRANGE(c, 'a', '{')) return c - 'a' + 10;
  if (INRANGE(c, '0', '9')) return c - '0';
  return 0;
}
/*******************************************************************/
static void hex_float(M_APM r, char *s)
{
  int bexp=0, d=0;
  char *bad, *point = NULL;
  M_set_to_zero(r);

  for(;;) {
    switch (d = *s++) {   // assume skipped 0x
      case '.': if (point) break;
        point = s;        // only 1 point allowed
        continue;
      case 'A' ... 'F': d |= 32;
      case 'a' ... 'f': d += 10 + '0' - 'a';
      case '0' ... '9':
        M_mul_digit(r, r, 16);
        M_add_digit(r, d - '0');
        continue;
      case 'p':
      case 'P':
        d = (*s == 0);    // bad if nothing after p
        bexp = strtol(s, &bad, 10);
        d |= *bad;        // bad if garbage after bexp
    }
    break;
  }
  if (d) puts("rpn: bad hex float");
  if (point) bexp -= (s - point - 1) * 4;
  m_apm_ishift(r, bexp);
}
/*******************************************************************/
static void do_func(struct opstr *cmd)
{
  switch(cmd->type & 0xf) {     // check stack
    case MM:  if (top > base) break;
              puts("rpn: stack empty"); return;
    case MMM: if (--top > base) break;
              puts("rpn: need x and y"); top++; return;
    case MMA: if (top++ < base + TOP) break;
              puts("rpn: stack full"); top--; return;
  }

  M_APM x = *top;
  switch(cmd->type) {           // apply function
    case MM:  cmd->func(*base, x); break;
    case MiM: cmd->func(*base, places, x); break;
    case MMM: cmd->func(*base, x, top[1]); break;
    case Mi2: cmd->func(*base, places, x, top[1]); break;
    case MMA: cmd->func(*base, top[-1]); break;
    case MiA: cmd->func(*base, places); break;
  }
  *top = *base;                 // swap result
  *base = x;
}
/*******************************************************************/
static char* clean(char *str)
{
  char *s = strchr(str, ',');   // remove all commas
  if (s == NULL) return str;    // nothing to do
  for(int i=0, j=1 ;; )
    if ((s[i] = s[j++]) != ',' && s[i++]==0) return str;
}
/*******************************************************************/
static void do_cmd(char *str)
{
  char *s = str;
  int i, c = (UCHAR) *s++;

  if ((i = lookup[c]) >= 0)     // possible cmd match
    do {
      UCHAR *s1 = (UCHAR *) s, *s2 = cmds[i].name+1;
      for(; *s1 == *s2; s1++, s2++)
        if (*s1 == 0) {do_func(&cmds[i]); return;}
    } while (c == cmds[++i].name[0]);

  switch (c) {          // special commands

    case 'h': puts(usage); return;
    case 'q': exit(0);  // quit

    case 'r':           // memory READ
      if (top == base + TOP) puts("rpn: stack full");
      else m_apm_copy(*++top, stack[mem_idx(*s)]);
      return;

    case 's':           // memory SAVE
      m_apm_copy(stack[mem_idx(*s)], *top);
      return;

    case 'm': {         // m+ or m-
      void (*f)() = m_apm_add;          // default = m+
      switch (*s) {
        case '-': f = m_apm_subtract;   // fall thru
        case '+': s++;                  // fall thru
      }
      i = mem_idx(*s);
      f(*base, stack[i], *top);         // add/subtract
      SWAP(M_APM, *base, stack[i]);
      return;
    }

    case 'k':           // kill stack
      i = mem_idx(*s);  // k = kill all
      top = (i==0 || top<=base+i) ? base : top - i;
      return;

    case '$':           // swap stack
      i = (*s-'s')*2+1; // $s = sorted, $r = reverse sorted
      if (top <= base + 1) {puts("rpn: need x and y"); return;}
      if (*s && abs(i)!=1) {puts("rpn: swap error"); return;}
      if (*s && (m_apm_compare(top[-1], top[0]) != i)) return;
      SWAP(M_APM, top[-1], top[0]);
      return;

    case '@':           // stack access: +1=top, -1=bottom
      if (top == base + TOP) {puts("rpn: stack full"); return;}
      i = mem_idx(s[*s == '-' || *s == '+']);
      if (++top, *s == '-') i = top - base - i;
      if (i != 0) m_apm_copy(*top, top[-i]);
      return;

    case '?':           // show stuff ...
      if (*s == '#') {puts(s+1); return;}       // skip '#'
      if (top == base) {puts("rpn: stack empty"); return;}
      switch(*s) {
        case 'i': show_rint(*top, s + 1); return;
        case 'm': show_mathematica(*top); return;
        case 'l': show_length(*top); return;
        case 's': for(M_APM *p=base; ++p < top;) show_number(*p);
                  // fall thru
        case '?': show_number(*top); return;    // no round
      }
      i = strtol(s, &s, 10);
      if (*s) puts("rpn: bad sig. digits");
      m_apm_round(*base, i>0? i-1 : places, *top);
      show_number(*base);
      return;

    // note: =#, f#, g# does inplace update

    case '=':           // round sig digits
      i = strtol(s, NULL, 10);  // set default if i>0
      m_apm_iround(*top, i>0 ? (places = i-1)
                      : i==0 ? places : -i-1); return;

    case 'f':           // fixpt rounding, negative OK
      m_apm_iround_fixpt(*top, strtol(s, NULL, 10)); return;

    case 'g':           // replace with IEEE double
      i = strtol(s, NULL, 10);  // assumed i >= 0
      M_set_double(*top, m_apm_to_double(*top), i-1); return;

    case '+':
    case '-': s++;      // skip sign for possibly hex-float
  }

  if (top == base + TOP) {
    puts("rpn: stack full");
  } else if ((s[0]|32)=='x' && s[-1]=='0') {
    hex_float(*++top, s+1);       // unsigned hex-float
    if (c == '-') (*top)->m_apm_sign *= -1; // add sign
  } else {
    M_set_string(*++top, str, &s);
    if (s == str) --top, printf("rpn: Bad command [%s]\n", s);
    else if (*s)  do_cmd(s);      // cmd after number
  }
}
/*******************************************************************/
static void do_file_cmds(FILE *f, int skip, int neg, char **post_cmds)
{
  int n = -skip;
  UCHAR *sp, *token;
  while(fgets(buf, sizeof buf, f)) {
    sp = (UCHAR *) clean(buf);
    do {
      while(lookup[*sp] >= (unsigned)-1) sp++;  // skip spaces
      if   (lookup[*sp] >= (unsigned)-3) break; // end-of-line
      token = sp++;
      while(lookup[*sp] <= (unsigned)-3) sp++;  // !space and !0
      if (*sp) *sp++ = 0;
      do_cmd((char *) token);
    } while (*sp);
    if (n++ >= 0)
      for(int i=neg; i; i++) do_cmd(post_cmds[i]);
  }
  m_apm_set_unsigned(base[-1], n+skip); // lines read -> Mem{
}
/*******************************************************************/
static void rpn_setup()
{
  setmode(STDIN_FILENO, _O_BINARY);
  setmode(STDOUT_FILENO, _O_BINARY);
  memset(lookup, -4, sizeof(lookup));   // build lookup table
  lookup[ '#'] = -3;
  lookup['\0'] = -2;
  lookup[ ' '] = lookup['\t'] = lookup['\n'] = -1;
  lookup['\v'] = lookup['\f'] = lookup['\r'] = -1;
  for(int i=ASIZE(cmds)-1; i--; ) lookup[cmds[i].name[0]] = i;
  for(int i=0; i<ASIZE(stack); i++) stack[i] = m_apm_init();
}
/*******************************************************************/
int main(int argc, char **argv)
{
  if (argc>1 && ISHELP(argv[1])) {puts(usage); return 0;}

  rpn_setup();
  argv[0] = "[";
  for(int i=(argc>1); i<argc; i++) {
    if (argv[i][0] == '#') continue;    // skip comments
    if (argv[i][0] != '[') {do_cmd(clean(argv[i])); continue;}
    int skip = mem_idx(argv[i][1]);     // parse post cmds
    int j = ++i;
    while(i<argc && argv[i][0]!=']') clean(argv[i++]) ;
    FILE *f = stdin;
    char *name = &argv[i][1];
    if (i<argc && *name && !(f = fopen(name, "rb")))
      return printf("rpn: bad file %s\n", name);
    do_file_cmds(f, skip, j - i, argv + i);
    if (f != stdin) fclose(f);
  }

  for(M_APM *p=base; ++p <= top;) {     // same as input-order
    m_apm_iround(*p, places);
    show_number(*p);
  }
  return 0;
}
