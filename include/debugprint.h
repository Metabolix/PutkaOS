#ifndef _DEBUGPRINT_H_
#define _DEBUGPRINT_H_ 1

#include <screen.h>
#define DEBUGP(str) (kprintf(__FILE__ ":%d: ", __LINE__)?print(str):0)
#define DEBUGF(...) (kprintf(__FILE__ ":%d: ", __LINE__)?kprintf(__VA_ARGS__):0)

#endif
