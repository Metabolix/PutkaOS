#include <math.h>
#include <string.h>

void asm_float_div_by_zero(void);
void asm_float_invalid(void);

#define DEF_WITH_1_PARAMS(type, name) \
	extern type name (type x)
#define DEF_WITH_2_PARAMS(type, name) \
	extern type name (type x, type y)

#define DEF_ALL(name, num) \
	DEF_WITH_ ## num ## _PARAMS (double, name); \
	DEF_WITH_ ## num ## _PARAMS (float, name ## f); \
	DEF_WITH_ ## num ## _PARAMS (long double, name ## l); \

DEF_ALL(pow_common, 2)
#if 0
+ pow(±0, y) returns ±∞ and raises the ‘‘divide-by-zero’’ ﬂoating-point exception for y an odd integer < 0.
+ pow(±0, y) returns +∞ and raises the ‘‘divide-by-zero’’ ﬂoating-point exception for y < 0 and not an odd integer.
+ pow(±0, y) returns ±0 for y an odd integer > 0.
+ pow(±0, y) returns +0 for y > 0 and not an odd integer.
+ pow(−1, ±∞) returns 1.
+ pow(+1, y) returns 1 for any y, even a NaN.
+ pow(x, ±0) returns 1 for any x, even a NaN.
+ pow(x, y) returns a NaN and raises the ‘‘invalid’’ ﬂoating-point exception for ﬁnite x < 0 and ﬁnite non-integer y.
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

static int is_int(long double x)
{
	if (0xfffffffeUL > -x && x < 0xffffffffeUL) return ((unsigned int)x == x);
	if (0xffffffffffffffeULL > -x && x < 0xfffffffffffffffeULL) return ((unsigned long long)x == x);
	return 1;
}
static int odd_int(long double x)
{
	if (!is_int(x)) return 0;
	if (0xffffffffffffffeULL > -x || x > 0xfffffffffffffffeULL) return 0;
	return ((unsigned long long)x & 1);
}

double pow(double x, double y) {return powl(x,y);}
float powf(float x, float y) {return powl(x,y);}
long double powl(long double x, long double y)
{
	const long double inf = 1.0 / 0.0;
	const long double ninf = 1.0 / 0.0;
	const long double zero = 0.0;
	const long double nzero = -0.0;
	const long double nan = 0.0 / 0.0;

	if (x == 0) {
		if (y < 0) {
			if (memcmp(&x, &zero, sizeof(long double)) == 0) {
				asm_float_div_by_zero();
				return inf;
			} else {
				asm_float_div_by_zero();
				return (odd_int(y) ? ninf : inf);
			}
		} else {
			if (memcmp(&x, &zero, sizeof(long double)) == 0) {
				return 0;
			} else {
				return (odd_int(y) ? nzero : zero);
			}
		}
	} else if (x == 1) {
		return 1;
	} else if (x == -1) {
		if (y == inf || y == ninf) {
			return 1;
		}
	} else if (y == 0) {
		return 1;
	} else if (y != y) {
		return y;
	} else if (x != x) {
		return x;
	} else if (y == ninf) {
		if (x < -1 || 1 < x) {
			return zero;
		} else {
			return inf;
		}
	} else if (y == inf) {
		if (x < -1 || 1 < x) {
			return inf;
		} else {
			return zero;
		}
	} else if (x < 0 && !is_int(y)) {
		asm_float_invalid();
		return nan;
	} else if (x == inf) {
		if (y < 0) {
			return zero;
		} else {
			return inf;
		}
	} else if (x == ninf) {
		if (y < 0) {
			return (odd_int(y) ? nzero : zero);
		} else {
			return (odd_int(y) ? ninf : inf);
		}
	}
	return pow_commonl(x, y);
}

#if 0
long double powl(long double x, long double y)
{
#if 0
	x
#endif
}
#endif
