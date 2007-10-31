#ifndef _DEBUG_H_
#define _DEBUG_H_ 1

#include <screen.h>
#include <timer.h>

#define DEBUGP(str) (kprintf(__FILE__ ": %s : %d:\n   >> ", __func__, __LINE__)?(print(str),kwait(0,100000),1):0)
#define DEBUGF(...) (kprintf(__FILE__ ": %s : %d:\n   >> ", __func__, __LINE__)?(kprintf(__VA_ARGS__),kwait(0,100000),1):0)

#endif
