#ifndef _MATH_H
#define _MATH_H 1

#define __MATH_OMAT_JUTUT 0

// C99 7.12 math.h

// TODO: Missing!

#define SELECT(x, func) \
	((sizeof((x)) == sizeof(float)) ? (func ## f)(x) : \
	((sizeof((x)) == sizeof(double)) ? (func)(x) : (func ## l)(x) ))

#define __MATH_RETVAL_F(T) T
#define __MATH_RETVAL_I(T) int
#define __MATH_RETVAL_L(T) long int
#define __MATH_RETVAL_Ll(T) long long int

#define __MATH_PARAMS_Cp(T)   const char * x
#define __MATH_PARAMS_F(T)    T x
#define __MATH_PARAMS_FF(T)   T x, T y
#define __MATH_PARAMS_FFl(T)  T x, long double y
#define __MATH_PARAMS_FFp(T)  T x, T * y
#define __MATH_PARAMS_FI(T)   T x, int y
#define __MATH_PARAMS_FIp(T)  T x, int * y
#define __MATH_PARAMS_FL(T)   T x, long int y
#define __MATH_PARAMS_FFF(T)  T x, T y, T z
#define __MATH_PARAMS_FFIp(T) T x, T y, int * z

#define __MATH_PASS_Cp   x
#define __MATH_PASS_F    x
#define __MATH_PASS_FF   x, y
#define __MATH_PASS_FFl  x, y
#define __MATH_PASS_FFp  x, y
#define __MATH_PASS_FI   x, y
#define __MATH_PASS_FIp  x, y
#define __MATH_PASS_FL   x, y
#define __MATH_PASS_FFF  x, y, z
#define __MATH_PASS_FFIp x, y, z

#define __MATH_USE_LD(ret, name, par) \
	__MATH_RETVAL_ ## ret (double) name (__MATH_PARAMS_ ## par (double)) { \
		return (__MATH_RETVAL_ ## ret (double)) \
		name ## l (__MATH_PASS_ ## par); \
	} \
	__MATH_RETVAL_ ## ret (float) name ## f (__MATH_PARAMS_ ## par (float)) { \
		return (__MATH_RETVAL_ ## ret (float)) \
		name ## l (__MATH_PASS_ ## par); \
	}

#define __MATH_REDIR(ret, T, name, new, par) \
	__MATH_RETVAL_ ## ret (T) name (__MATH_PARAMS_ ## par (T)) { \
		return new(__MATH_PASS_ ## par); \
	}

#define __MATH_REDIR_ALL(ret, name, new, par) \
	__MATH_EXTERN_ALL(ret, new, par) \
	__MATH_REDIR(ret, long double, name ## l, new ## l, par) \
	__MATH_REDIR(ret, double,      name,      new,      par) \
	__MATH_REDIR(ret, float,       name ## f, new ## f, par)

#define __MATH_REDIR_ALL_LD(ret, name, new, par) \
	__MATH_EXTERN_LD(ret, new, par) \
	__MATH_REDIR(ret, long double, name ## l, new ## l, par) \
	__MATH_REDIR(ret, double,      name,      new ## l, par) \
	__MATH_REDIR(ret, float,       name ## f, new ## l, par)

#define __MATH_EXTERN_DN(ret, name, par) \
	extern __MATH_RETVAL_ ## ret (double) name (__MATH_PARAMS_ ## par (double));

#define __MATH_EXTERN_FN(ret, name, par) \
	extern __MATH_RETVAL_ ## ret (float) name (__MATH_PARAMS_ ## par (float));

#define __MATH_EXTERN_LDN(ret, name, par) \
	extern __MATH_RETVAL_ ## ret (long double) name (__MATH_PARAMS_ ## par (long double));

#define __MATH_EXTERN_D(ret, name, par) __MATH_EXTERN_DN(ret, name, par)
#define __MATH_EXTERN_F(ret, name, par) __MATH_EXTERN_FN(ret, name ## f, par)
#define __MATH_EXTERN_LD(ret, name, par) __MATH_EXTERN_LDN(ret, name ## l, par)

#define __MATH_EXTERN_ALL(ret, name, par) \
	__MATH_EXTERN_LD(ret, name, par) \
	__MATH_EXTERN_D(ret, name, par) \
	__MATH_EXTERN_F(ret, name, par)

// 2: float_t, double_t

#if !defined(FLT_EVAL_METHOD) || (FLT_EVAL_METHOD == 0)
typedef float float_t;
typedef double double_t;
#elif (FLT_EVAL_METHOD == 1)
typedef double float_t;
typedef double double_t;
#elif (FLT_EVAL_METHOD == 2)
typedef long double float_t;
typedef long double double_t;
#endif

// 3: HUGE_VAL
// 4: INFINITY
#define INFINITY ((float)(1.0f/0.0f))
#define HUGE_VAL ((double)(INFINITY))
#define HUGE_VALF ((float)(INFINITY))
#define HUGE_VALL ((long double)(INFINITY))

// 5: NAN (quiet NaN) Missing!
// #define NAN ((float)())

extern int __fpclassify_fxam(int fxam);
extern int __signbit_fxam(int fxam);

// TODO:
__MATH_EXTERN_ALL(I, asm_fxam, F)
__MATH_EXTERN_ALL(F, fabs, F)
__MATH_EXTERN_ALL(F, rint, F)
__MATH_EXTERN_ALL(F, logb, F)
__MATH_EXTERN_ALL(F, exp2, F)
__MATH_EXTERN_ALL(F, log2, F)
__MATH_EXTERN_ALL(F, log10, F)
__MATH_EXTERN_ALL(F, pow, FF)

// 6: number classification macros

#define FP_CLASS_UNS (1 << 0)
#define FP_CLASS_NAN (1 << 1)
#define FP_CLASS_FIN (1 << 2) /* (. | FP_CLASS_DEN) */
#define FP_CLASS_INF (1 << 3)
#define FP_CLASS_ZER (1 << 4)
#define FP_CLASS_EMP (1 << 5)
#define FP_CLASS_DEN (1 << 6)
#define FP_CLASS_ERR (1 << 7)
#define FP_CLASS_NOR (FP_CLASS_FIN)

#define FP_INFINITE FP_CLASS_FIN
#define FP_NAN FP_CLASS_NAN
#define FP_NORMAL FP_CLASS_NOR
#define FP_SUBNORMAL FP_CLASS_DEN
#define FP_ZERO FP_CLASS_ZER

// 7: FP_FAST_FMA(F|L) Missing!

// 8: macros for ilogb
#define FP_ILOGB0 (INT_MIN)
#define FP_ILOGBNAN (INT_MAX)

// 9: error macros, MATH_ERRNO, MATH_ERREXCEPT, (int)math_errhandling Missing!

// TODO:
#define fpclassify(x) (__fpclassify_fxam(SELECT((x), asm_fxam)))

#define isfinite(x) (fpclassify(x) & (FP_CLASS_NOR | FP_CLASS_DEN | FP_CLASS_ZER))
#define isnan(x) (fpclassify(x) & FP_CLASS_NAN)
#define isinf(x) (fpclassify(x) & FP_CLASS_INF)
#define isnormal(x) (fpclassify(x) & FP_CLASS_NOR)

#define signbit(x) (__signbit_fxam(SELECT((x), asm_fxam)))

#if __MATH_OMAT_JUTUT
#define isunsupported(x) (fpclassify(x) & FP_CLASS_UNS)
#define isdenormal(x) (fpclassify(x) & FP_CLASS_DEN)
#define iszero(x) (fpclassify(x) & FP_CLASS_ZER)
#define isempty(x) (fpclassify(x) & FP_CLASS_EMP)
#endif

// #pragma STDC FP_CONTRACT on-off-switch Missing!

__MATH_EXTERN_ALL(F, acos, F)
__MATH_EXTERN_ALL(F, asin, F)
__MATH_EXTERN_ALL(F, atan, F)
__MATH_EXTERN_ALL(F, atan2, FF)

__MATH_EXTERN_ALL(F, cos, F)
__MATH_EXTERN_ALL(F, sin, F)
__MATH_EXTERN_ALL(F, tan, F)

__MATH_EXTERN_ALL(F, acosh, F)
__MATH_EXTERN_ALL(F, asinh, F)
__MATH_EXTERN_ALL(F, atanh, F)

__MATH_EXTERN_ALL(F, cosh, F)
__MATH_EXTERN_ALL(F, sinh, F)
__MATH_EXTERN_ALL(F, tanh, F)

__MATH_EXTERN_ALL(F, exp, F)
__MATH_EXTERN_ALL(F, exp2, F)
__MATH_EXTERN_ALL(F, expm1, F)

__MATH_EXTERN_ALL(F, frexp, FIp)

__MATH_EXTERN_ALL(I, ilogb, F)

__MATH_EXTERN_ALL(F, ldexp, FI)

__MATH_EXTERN_ALL(F, log, F)
__MATH_EXTERN_ALL(F, log10, F)
__MATH_EXTERN_ALL(F, log1p, F)
__MATH_EXTERN_ALL(F, log2, F)
__MATH_EXTERN_ALL(F, logb, F)

__MATH_EXTERN_ALL(F, modf, FFp)
__MATH_EXTERN_ALL(F, scalbn, FI)
__MATH_EXTERN_ALL(F, scalbln, FL)

__MATH_EXTERN_ALL(F, cbrt, F)
__MATH_EXTERN_ALL(F, fabs, F)

__MATH_EXTERN_ALL(F, hypot, FF)
__MATH_EXTERN_ALL(F, pow, FF)

__MATH_EXTERN_ALL(F, sqrt, F)

__MATH_EXTERN_ALL(F, erf, F)
__MATH_EXTERN_ALL(F, erfc, F)
__MATH_EXTERN_ALL(F, lgamma, F)
__MATH_EXTERN_ALL(F, tgamma, F)

__MATH_EXTERN_ALL(F, ceil, F)
__MATH_EXTERN_ALL(F, floor, F)
__MATH_EXTERN_ALL(F, nearbyint, F)

__MATH_EXTERN_ALL(F, rint, F)
__MATH_EXTERN_ALL(L, lrint, F)
__MATH_EXTERN_ALL(Ll, llrint, F)

__MATH_EXTERN_ALL(F, round, F)
__MATH_EXTERN_ALL(L, lround, F)
__MATH_EXTERN_ALL(Ll, llround, F)

__MATH_EXTERN_ALL(F, trunc, F)

__MATH_EXTERN_ALL(F, fmod, FF)
__MATH_EXTERN_ALL(F, remainder, FF)
__MATH_EXTERN_ALL(F, remquo, FFIp)

__MATH_EXTERN_ALL(F, copysign, FF)

__MATH_EXTERN_ALL(F, nan, Cp)

__MATH_EXTERN_ALL(F, nextafter, FF)
__MATH_EXTERN_ALL(F, nexttoward, FFl)

__MATH_EXTERN_ALL(F, fdim, FF)
__MATH_EXTERN_ALL(F, fmax, FF)
__MATH_EXTERN_ALL(F, fmin, FF)
__MATH_EXTERN_ALL(F, fma, FFF)

/*
TODO:
Funktio määrittämiseen, (SELECT),
((int)((funktio(x, y) & VAKIO) ? 1 : 0))

int isgreater(real-floating x, real-floating y);
int isgreaterequal(real-floating x, real-floating y);
int isless(real-floating x, real-floating y);
int islessequal(real-floating x, real-floating y);
int islessgreater(real-floating x, real-floating y);
int isunordered(real-floating x, real-floating y);
*/

#endif
