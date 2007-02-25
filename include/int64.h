#ifndef _INT64_H_
#define _INT64_H_ 1

#include <stdint.h>

typedef union {
	uint64_t u64;
	struct {uint32_t lo, hi;} u32;
	struct {uint16_t low, mlow, mhiw, hiw;} u16;
} uni_uint64_t;

#endif
