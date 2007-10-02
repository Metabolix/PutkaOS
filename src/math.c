#include <math.h>
#include <string.h>

extern void asm_float_div_by_zero(void);
extern void asm_float_invalid(void);

int __fpclassify_fxam(int fxam)
{
	fxam = (fxam & 1) | ((fxam >> 1) & 6);
	switch (fxam) {
		case 0: return FP_CLASS_UNS;
		case 1: return FP_CLASS_NAN;
		case 2: return FP_CLASS_FIN;
		case 3: return FP_CLASS_INF;
		case 4: return FP_CLASS_ZER;
		case 5: return FP_CLASS_EMP;
		case 6: return FP_CLASS_DEN;
	}
	return FP_CLASS_ERR;
}
int __signbit_fxam(int fxam)
{
	return fxam & 2;
}

__MATH_EXTERN_ALL(F, asm_f2xm1, F)
__MATH_EXTERN_ALL(I, asm_is_integer, F)
__MATH_EXTERN_ALL(I, asm_is_odd_integer, F)
__MATH_EXTERN_ALL(F, asm_fabs, F)
__MATH_EXTERN_ALL(F, asm_log2, F)
__MATH_EXTERN_ALL(F, asm_exp2, F)
__MATH_EXTERN_ALL(F, asm_acos, F)


// TODO: (F, acos, F)
__MATH_REDIR_ALL(F, acos, asm_acos, F)

// TODO: (F, asin, F)
__MATH_REDIR_ALL(F, asin, asm_asin, F)

// TODO: (F, atan, F)
__MATH_REDIR_ALL(F, atan, asm_atan, F)

// TODO: (F, atan2, FF)
__MATH_REDIR_ALL(F, atan2, asm_atan2, FF)

// TODO: (F, cos, F)
__MATH_REDIR_ALL(F, cos, asm_cos, F)

// TODO: (F, sin, F)
__MATH_REDIR_ALL(F, sin, asm_sin, F)

// TODO: (F, tan, F)
__MATH_REDIR_ALL(F, tan, asm_tan, F)

// TODO: (F, acosh, F)
// TODO: (F, asinh, F)
// TODO: (F, atanh, F)

// TODO: (F, cosh, F)
// TODO: (F, sinh, F)
// TODO: (F, tanh, F)

// TODO: (F, exp, F)
__MATH_REDIR_ALL(F, exp, asm_exp, F)

// TODO: (F, exp2, F)
__MATH_REDIR_ALL(F, exp2, asm_exp2, F)

// TODO: (F, expm1, F)


// TODO: (F, frexp, FIp)
__MATH_REDIR_ALL(F, frexp, asm_frexp, FIp)

// TODO: (I, ilogb, F)
__MATH_REDIR_ALL(I, ilogb, asm_ilogb, F)

// TODO: (F, ldexp, FI)
__MATH_REDIR_ALL(F, ldexp, asm_fscalei, FI)

// TODO: (F, log, F)
__MATH_REDIR_ALL(F, log, asm_ln, F)

// TODO: (F, log10, F)
__MATH_REDIR_ALL(F, log10, asm_log10, F)

// TODO: (F, log1p, F)

// TODO: (F, log2, F)
__MATH_REDIR_ALL(F, log2, asm_log2, F)

// TODO: (F, logb, F)
__MATH_REDIR_ALL(F, logb, asm_logb, F)

// TODO: (F, modf, FFp)

// TODO: (F, scalbn, FI)
__MATH_REDIR_ALL(F, scalbn, asm_fscalei, FI)

// TODO: (F, scalbln, FL)
__MATH_REDIR_ALL(F, scalbln, asm_fscalel, FL)

// TODO: (F, cbrt, F)

// TODO: (F, fabs, F)
__MATH_REDIR_ALL(F, fabs, asm_fabs, F)

// TODO: (F, hypot, FF)
__MATH_REDIR_ALL(F, hypot, asm_hypot, FF)

// TODO: (F, pow, FF)
__MATH_USE_LD(F, pow, FF)

// TODO: (F, sqrt, F)
__MATH_REDIR_ALL(F, sqrt, asm_sqrt, F)

// TODO: (F, erf, F)
// TODO: (F, erfc, F)
// TODO: (F, lgamma, F)
// TODO: (F, tgamma, F)

// TODO: (F, ceil, F)
// TODO: (F, floor, F)
// TODO: (F, nearbyint, F)

// TODO: (F, rint, F)
// TODO: (L, lrint, F)
// TODO: (Ll, llrint, F)

// TODO: (F, round, F)
// TODO: (L, lround, F)
// TODO: (Ll, llround, F)

// TODO: (F, trunc, F)

// TODO: (F, fmod, FF)
// TODO: (F, remainder, FF)
// TODO: (F, remquo, FFIp)

// TODO: (F, copysign, FF)

// TODO: (F, nan, Cp)

// TODO: (F, nextafter, FF)
// TODO: (F, nexttoward, FFl)

// TODO: (F, fdim, FF)
// TODO: (F, fmax, FF)
// TODO: (F, fmin, FF)
// TODO: (F, fma, FFF)

#if 0
+ pow(±0, y) returns ±∞ and raises the ‘‘divide-by-zero’’ floating-point exception for y an odd integer < 0.
+ pow(±0, y) returns +∞ and raises the ‘‘divide-by-zero’’ floating-point exception for y < 0 and not an odd integer.
+ pow(±0, y) returns ±0 for y an odd integer > 0.
+ pow(±0, y) returns +0 for y > 0 and not an odd integer.
+ pow(−1, ±∞) returns 1.
+ pow(+1, y) returns 1 for any y, even a NaN.
+ pow(x, ±0) returns 1 for any x, even a NaN.
+ pow(x, y) returns a NaN and raises the ‘‘invalid’’ floating-point exception for finite x < 0 and finite non-integer y.
+ pow(x, −∞) returns +∞ for | x | < 1.
+ pow(x, −∞) returns +0 for | x | > 1.
+ pow(x, +∞) returns +0 for | x | < 1.
+ pow(x, +∞) returns +∞ for | x | > 1.
pow(−∞, y) returns −0 for y an odd integer < 0.
pow(−∞, y) returns +0 for y < 0 and not an odd integer.
pow(−∞, y) returns −∞ for y an odd integer > 0.
pow(−∞, y) returns +∞ for y > 0 and not an odd integer.
pow(+∞, y) returns +0 for y < 0.
pow(+∞, y) returns +∞ for y > 0.
#endif

long double powl(long double x, long double y)
{
	const long double inf = 1.0 / 0.0;
	const long double ninf = 1.0 / 0.0;
	const long double zero = 0.0;
	const long double nzero = -0.0;
	const long double nan = 0.0 / 0.0;

	// pow(x, ±0) returns 1 for any x, even a NaN.
	if (y == 0) {
		return 1;
	}
	// pow(+1, y) returns 1 for any y, even a NaN.
	if (x == 1) {
		return 1;
	}
	// TODO: Kai muut NaN-hommat ovat NaN?
	if (isnan(x) || isnan(y)) {
		asm_float_invalid();
		return nan;
	}
	if (isinf(y)) {
		// pow(−1, ±∞) returns 1.
		if (x == -1) {
			return 1;
		}
		if ((signbit(y) != 0) ^ (fabsl(x) < 1)) {
			// pow(x, −∞) returns +0 for | x | > 1.
			// pow(x, +∞) returns +0 for | x | < 1.
			return zero;
		}
		// pow(x, −∞) returns +∞ for | x | < 1.
		// pow(x, +∞) returns +∞ for | x | > 1.
		return inf;
	}
	if (x == 0) {
		if (!signbit(y)) {
			// pow(±0, y) returns ±0 for y an odd integer > 0.
			if (asm_is_odd_integerl(y)) {
				return x;
			}
			// pow(±0, y) returns +0 for y > 0 and not an odd integer.
			return +0.0;
		}
		// pow(±0, y) returns ±∞ and raises the ‘‘divide-by-zero’’ floating-point exception for y an odd integer < 0.
		if (asm_is_odd_integerl(y)) {
			asm_float_div_by_zero();
			return signbit(x) ? ninf : inf;
		}
		// pow(±0, y) returns +∞ and raises the ‘‘divide-by-zero’’ floating-point exception for y < 0 and not an odd integer.
		asm_float_div_by_zero();
		return inf;
	}
	// pow(x, y) returns a NaN and raises the ‘‘invalid’’ floating-point exception for finite x < 0 and finite non-integer y.
	if (isfinite(x) && (x < 0) && !asm_is_integerl(y)) {
		asm_float_invalid();
		return nan;
	}
	if (isinf(x)) {
		if (!signbit(x)) {
			// pow(+∞, y) returns +0 for y < 0.
			if (signbit(y)) {
				return zero;
			}
			// pow(+∞, y) returns +∞ for y > 0.
			return inf;
		}

		if (!signbit(y)) {
			// pow(−∞, y) returns −∞ for y an odd integer > 0.
			if (asm_is_odd_integerl(y)) {
				return ninf;
			}
			// pow(−∞, y) returns +∞ for y > 0 and not an odd integer.
			return inf;
		}
		// pow(−∞, y) returns −0 for y an odd integer < 0.
		if (asm_is_odd_integerl(y)) {
			return nzero;
		}
		// pow(−∞, y) returns +0 for y < 0 and not an odd integer.
		return zero;
	}

	return exp2l(y * log2l(x));
}

