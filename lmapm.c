// gcc -c lmapm.c -Ic:/ -DDLUA_BUILD_AS_DLL -O2
// gcc -shared -o mapm.dll lmapm.o d:/lua/lua51.dll -Lc:/mapm -lmapm

/*
* lmapm.c
* big-number library for Lua 5.1 based on the MAPM library
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 19 Apr 2010 09:05:40
* This code is hereby placed in the public domain.
*/

#include <stdlib.h>

#include "mapm/m_apm.h"

#include "luajit/src/lua.h"
#include "luajit/src/lauxlib.h"

#define lua_boxpointer(L,u) \
	(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))

#define MYNAME		"mapm"
#define MYVERSION	MYNAME " " MAPM_LIB_SHORT_VERSION " library for " LUA_VERSION
#define MYTYPE		MYNAME " bignumber"

static int PLACES=39;   // default = 40 sig. digits
static lua_State *LL=NULL;

static M_APM Bnew(lua_State *L)
{
 M_APM x=m_apm_init();
 lua_boxpointer(L,x);
 luaL_getmetatable(L,MYTYPE);
 lua_setmetatable(L,-2);
 return x;
}

static M_APM Bget(lua_State *L, int i)
{
 LL=L;
 switch (lua_type(L,i))     // bad input: number(x) = nil
 {
  case LUA_TNUMBER:
  {
   M_APM x=Bnew(L);
   double val = lua_tonumber(L, i);
   if (isfinite(val)) M_set_double(x, val, PLACES);
   else {lua_remove(L, -1); lua_pushnil(L);}
   lua_replace(L,i);
   return x;
  }
  case LUA_TSTRING:
  {
   M_APM x=Bnew(L);
   char *start, *end;
   M_set_string(x, start = (char*) lua_tostring(L, i), &end);
   if (start == end) {lua_remove(L, -1); lua_pushnil(L);}
   lua_replace(L,i);
   return x;
  }
  default:
   return *((void**)luaL_checkudata(L,i,MYTYPE));
 }
}

static int Bdo0(lua_State *L, void (*f)(M_APM y, M_APM x))
{
 M_APM a=Bget(L,1);
 M_APM c=Bnew(L);
 f(c,a);
 return 1;
}

static int Bdo1(lua_State *L, void (*f)(M_APM y, int n, M_APM x))
{
 int n=luaL_optint(L,2,PLACES);
 M_APM a=Bget(L,1);
 M_APM c=Bnew(L);
 f(c,n,a);
 return 1;
}

static int Bdo2(lua_State *L, void (*f)(M_APM z, M_APM x, M_APM y))
{
 M_APM a=Bget(L,1);
 M_APM b=Bget(L,2);
 M_APM c=Bnew(L);
 f(c,a,b);
 return 1;
}

static int Bdo3(lua_State *L, void (*f)(M_APM z, int n, M_APM x, M_APM y))
{
 int n=luaL_optint(L,3,PLACES);
 M_APM a=Bget(L,1);
 M_APM b=Bget(L,2);
 M_APM c=Bnew(L);
 f(c,n,a,b);
 return 1;
}

static int Bplaces(lua_State *L)		/** places([n]) */
{
 lua_pushnumber(L,PLACES);          // old places returned
 PLACES=luaL_optint(L,1,PLACES);
 return 1;
}

static int Btostring(lua_State *L)
{
 M_APM a=Bget(L,1);
 int n=luaL_optint(L,2,PLACES);     // tostring(x, [places])
 int m = (n==-1) ? a->m_apm_datalength : abs(n);
 char *s = malloc(m + 16);
 if (!s) M_apm_error(M_APM_FATAL, "tostring: out of memory");
 m_apm_to_string(s,n,a);
 lua_pushstring(L,s);
 free(s);
 return 1;
}

static int Btofixed(lua_State *L)
{
 M_APM a=Bget(L,1);
 int n=luaL_optint(L,2,PLACES);     // tofixed(x, [places])
 char *s = m_apm_to_fixpt_stringexp(n,a,'.',0,0);
 lua_pushstring(L,s);
 free(s);
 return 1;
}

static int Btonumber(lua_State *L)		/** tonumber(x) */
{
 lua_pushnumber(L, m_apm_to_double(Bget(L, 1)));
 return 1;
}

static int Bnumber(lua_State *L)		/** number(x) */
{
 Bget(L,1);
 lua_settop(L,1);
 return 1;
}

static int Bround(lua_State *L)			/** round(x) */
{
 return Bdo1(L,m_apm_round);
}

static int Binv(lua_State *L)			/** inv(x) */
{
 return Bdo1(L,m_apm_reciprocal);
}

static int Bsqrt(lua_State *L)			/** sqrt(x) */
{
 return Bdo1(L,m_apm_sqrt);
}

static int Bcbrt(lua_State *L)			/** cbrt(x) */
{
 return Bdo1(L,m_apm_cbrt);
}

static int Blog(lua_State *L)			/** log(x) */
{
 return Bdo1(L,m_apm_log);
}

static int Blog10(lua_State *L)			/** log10(x) */
{
 return Bdo1(L,m_apm_log10);
}

static int Bexp(lua_State *L)			/** exp(x) */
{
 return Bdo1(L,m_apm_exp);
}

static int Bsin(lua_State *L)			/** sin(x) */
{
 return Bdo1(L,m_apm_sin);
}

static int Bcos(lua_State *L)			/** cos(x) */
{
 return Bdo1(L,m_apm_cos);
}

static int Btan(lua_State *L)			/** tan(x) */
{
 return Bdo1(L,m_apm_tan);
}

static int Basin(lua_State *L)			/** asin(x) */
{
 return Bdo1(L,m_apm_asin);
}

static int Bacos(lua_State *L)			/** acos(x) */
{
 return Bdo1(L,m_apm_acos);
}

static int Batan(lua_State *L)			/** atan(x) */
{
 return Bdo1(L,m_apm_atan);
}

static int Bsincos(lua_State *L)		/** sincos(x) */
{
 int n=luaL_optint(L,2,PLACES);
 M_APM a=Bget(L,1);
 M_APM s=Bnew(L);
 M_APM c=Bnew(L);
 m_apm_sin_cos(s,c,n,a);
 return 2;
}

static int Batan2(lua_State *L)			/** atan2(y,x) */
{
 return Bdo3(L,m_apm_atan2);
}

static int Bsinh(lua_State *L)			/** sinh(x) */
{
 return Bdo1(L,m_apm_sinh);
}

static int Bcosh(lua_State *L)			/** cosh(x) */
{
 return Bdo1(L,m_apm_cosh);
}

static int Btanh(lua_State *L)			/** tanh(x) */
{
 return Bdo1(L,m_apm_tanh);
}

static int Basinh(lua_State *L)			/** asinh(x) */
{
 return Bdo1(L,m_apm_asinh);
}

static int Bacosh(lua_State *L)			/** acosh(x) */
{
 return Bdo1(L,m_apm_acosh);
}

static int Batanh(lua_State *L)			/** atanh(x) */
{
 return Bdo1(L,m_apm_atanh);
}

static int Babs(lua_State *L)			/** abs(x) */
{
 return Bdo0(L,m_apm_absolute_value);
}

static int Bneg(lua_State *L)			/** neg(x) */
{
 return Bdo0(L,m_apm_negate);
}

static int Bfactorial(lua_State *L)		/** factorial(x) */
{
 return Bdo0(L,m_apm_factorial);
}

static int Bfloor(lua_State *L)			/** floor(x) */
{
 return Bdo0(L,m_apm_floor);
}

static int Bceil(lua_State *L)			/** ceil(x) */
{
 return Bdo0(L,m_apm_ceil);
}

static int Badd(lua_State *L)			/** add(x,y) */
{
 return Bdo2(L,m_apm_add);
}

static int Bsub(lua_State *L)			/** sub(x,y) */
{
 return Bdo2(L,m_apm_subtract);
}

static int Bmul(lua_State *L)			/** mul(x,y) */
{
 return Bdo2(L,m_apm_multiply);
}

static int Bdiv(lua_State *L)			/** div(x,y) */
{
 return Bdo3(L,m_apm_divide);
}

static int Bidiv(lua_State *L)			/** idiv(x,y) */
{
 M_APM a=Bget(L,1);
 M_APM b=Bget(L,2);
 M_APM q=Bnew(L);
 M_APM r=Bnew(L);
 m_apm_integer_div_rem(q,r,a,b);
 return 2;
}

static int Bmod(lua_State *L)			/** mod(x,y) */
{
 Bidiv(L);
 return 1;
}

static int Bpow(lua_State *L)			/** pow(x,y) */
{
 return Bdo3(L,m_apm_pow);
}

static int Bcompare(lua_State *L)		/** compare(x,y) */
{
 M_APM a=Bget(L,1);
 M_APM b=Bget(L,2);
 lua_pushinteger(L,m_apm_compare(a,b));
 return 1;
}

static int Beq(lua_State *L)
{
 M_APM a=Bget(L,1);
 M_APM b=Bget(L,2);
 lua_pushboolean(L,m_apm_compare(a,b)==0);
 return 1;
}

static int Blt(lua_State *L)
{
 M_APM a=Bget(L,1);
 M_APM b=Bget(L,2);
 lua_pushboolean(L,m_apm_compare(a,b)<0);
 return 1;
}

static int Bsign(lua_State *L)			/** sign(x) */
{
 M_APM a=Bget(L,1);
 lua_pushinteger(L,m_apm_sign(a));
 return 1;
}

static int Bexponent(lua_State *L)		/** exponent(x) */
{
 M_APM a=Bget(L,1);
 lua_pushinteger(L,m_apm_exponent(a));
 return 1;
}

static int Bisint(lua_State *L)			/** isint(x) */
{
 M_APM a=Bget(L,1);
 lua_pushboolean(L,m_apm_is_integer(a));
 return 1;
}

static int Biseven(lua_State *L)		/** iseven(x) */
{
 M_APM a=Bget(L,1);
 lua_pushboolean(L,m_apm_is_integer(a) && m_apm_is_even(a));
 return 1;
}

static int Bisodd(lua_State *L)			/** isodd(x) */
{
 M_APM a=Bget(L,1);
 lua_pushboolean(L,m_apm_is_integer(a) && m_apm_is_odd(a));
 return 1;
}

static int Bdigits(lua_State *L)		/** digits(x) */
{
 M_APM a=Bget(L,1);
 lua_pushinteger(L,m_apm_significant_digits(a));
 return 1;
}

static int Bgc(lua_State *L)
{
 M_APM a=Bget(L,1);
 m_apm_free(a);
 lua_pushnil(L);
 lua_setmetatable(L,1);
 return 0;
}

static int Bpowmod(lua_State *L)
{
 M_APM x=Bget(L,1);
 M_APM n=Bget(L,2);
 M_APM m=Bget(L,3);

 if (x->m_apm_datalength > x->m_apm_exponent ||
     m->m_apm_datalength > m->m_apm_exponent ||
     n->m_apm_datalength > n->m_apm_exponent ||
     n->m_apm_sign < 0)     // inverse not implemented
   luaL_error(L, "powmod require nonzero integer input");

 M_APM z=Bnew(L);
 m_apm_powmod(z, x, n, m);
 return 1;
}

static const luaL_Reg R[] =
{
	{ "__add",	Badd	},
	{ "__div",	Bdiv	},
	{ "__eq",	Beq	},
	{ "__gc",	Bgc	},
	{ "__lt",	Blt	},
	{ "__mod",	Bmod	},
	{ "__mul",	Bmul	},
	{ "__pow",	Bpow	},
	{ "__sub",	Bsub	},
	{ "__tostring",	Btostring},
	{ "__unm",	Bneg	},
	{ "abs",	Babs	},
	{ "acos",	Bacos	},
	{ "acosh",	Bacosh	},
	{ "add",	Badd	},
	{ "asin",	Basin	},
	{ "asinh",	Basinh	},
	{ "atan",	Batan	},
	{ "atan2",	Batan2	},
	{ "atanh",	Batanh	},
	{ "cbrt",	Bcbrt	},
	{ "ceil",	Bceil	},
	{ "compare",	Bcompare},
	{ "cos",	Bcos	},
	{ "cosh",	Bcosh	},
	{ "digits",	Bdigits},
	{ "div",	Bdiv	},
	{ "exp",	Bexp	},
	{ "exponent",	Bexponent},
	{ "factorial",	Bfactorial},
	{ "floor",	Bfloor	},
	{ "idiv",	Bidiv	},
	{ "inv",	Binv	},
	{ "iseven",	Biseven	},
	{ "isint",	Bisint	},
	{ "isodd",	Bisodd	},
	{ "log",	Blog	},
	{ "log10",	Blog10	},
	{ "mod",	Bmod	},
	{ "mul",	Bmul	},
	{ "neg",	Bneg	},
	{ "number",	Bnumber	},
	{ "places",	Bplaces	},
	{ "pow",	Bpow	},
	{ "powmod",	Bpowmod},
	{ "round",	Bround	},
	{ "sign",	Bsign	},
	{ "sin",	Bsin	},
	{ "sincos",	Bsincos	},
	{ "sinh",	Bsinh	},
	{ "sqrt",	Bsqrt	},
	{ "sub",	Bsub	},
	{ "tan",	Btan	},
	{ "tanh",	Btanh	},
	{ "tofixed",	Btofixed},
	{ "tonumber",	Btonumber},
	{ "tostring",	Btostring},
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_mapm(lua_State *L)
{
 luaL_newmetatable(L,MYTYPE);
 lua_setglobal(L,MYNAME);
 luaL_register(L,MYNAME,R);
 lua_pushliteral(L,"version");			/** version */
 lua_pushliteral(L,MYVERSION);
 lua_settable(L,-3);
 lua_pushliteral(L,"__index");
 lua_pushvalue(L,-2);
 lua_settable(L,-3);
 return 1;
}
