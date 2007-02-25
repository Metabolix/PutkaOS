#include <int64.h>

int64_t __divdi3(const int64_t a, const int64_t b);
uint64_t __udivdi3(const uint64_t a, const uint64_t b);
int64_t __moddi3(const int64_t a, const int64_t b);
uint64_t __umoddi3(const uint64_t a, const uint64_t b);
uint64_t __uint64_div_rem(uni_uint64_t a, uni_uint64_t b, uint64_t *rem);

uint64_t uint64_div_rem(uint64_t a, uint64_t b, uint64_t *rem);
__asm__("uint64_div_rem:\n    jmp __uint64_div_rem\n");

int64_t __divdi3(const int64_t a, const int64_t b)
{
	uint64_t au, bu;
	au = (a < 0) ? -a : a;
	bu = (b < 0) ? -b : b;
	return ((a < 0) ^ (b < 0)) ? -uint64_div_rem(au, bu, 0) : uint64_div_rem(au, bu, 0);
}

uint64_t __udivdi3(const uint64_t a, const uint64_t b)
{
	return uint64_div_rem(a, b, 0);
}

int64_t __moddi3(const int64_t a, const int64_t b)
{
	uint64_t au, bu, retval;
	au = (a < 0) ? -a : a;
	bu = (b < 0) ? -b : b;
	uint64_div_rem(au, bu, &retval);
	if (a < 0) {
		return -retval;
	}
	return retval;
}

uint64_t __umoddi3(const uint64_t a, const uint64_t b)
{
	uint64_t retval;
	uint64_div_rem(a, b, &retval);
	return retval;
}

uint64_t __uint64_div_rem(uni_uint64_t a, uni_uint64_t b, uint64_t *rem)
{
	#define RETURN(res_i, rem_i); {if(rem){*rem=(rem_i);}return(res_i);}
	uint64_t result, yla, ala, yla2, ala2;
	uint32_t i, j, kikkare = 0;
	if (!b.u64) {
		static volatile const unsigned int one = 1, zero = 0;
		RETURN(a.u64, one/zero);
	}
	if (a.u32.hi >> 31) {
		if (b.u32.lo & 1) {
			kikkare = a.u32.lo & 1;
			a.u64 >>= 1;
			result = __uint64_div_rem(a, b, &ala);
			result <<= 1;
			ala <<= 1;
			ala += kikkare;
			RETURN(result, ala);
		} else {
			a.u64 >>= 1;
			b.u64 >>= 1;
		}
	}
	if (!a.u32.hi) {
		if (!b.u32.hi) {
			RETURN(a.u32.lo / b.u32.lo, a.u32.lo % b.u32.lo);
		} else {
			RETURN(0, a.u32.lo);
		}
	} else {
		if (b.u32.hi) {
			if (b.u32.hi > a.u32.hi) {
				RETURN(0, a.u64);
			}
			for (i = b.u32.hi, j = 32; i; i >>= 1, ++j);
		} else {
			for (i = b.u32.lo, j = 0; i; i >>= 1, ++j);
		}
		yla = a.u64 >> (j-1);
		ala = a.u64 >> j;
		while (yla > ala + 1) {
			result = (yla + ala) >> 1;
			if (result * b.u64 > a.u64) {
				yla = result;
			} else {
				if (result * b.u64 == a.u64) {
					RETURN(result, 0);
				}
				ala = result;
			}
		}
		yla2 = yla * b.u64;
		ala2 = ala * b.u64;
		if (yla2 > a.u64 || ala2 > yla2) {
			RETURN(ala, a.u64 - ala2);
		}
		RETURN(yla, a.u64 - yla2);
	}
	#undef RETURN
}
