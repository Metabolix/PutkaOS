#ifndef _DEBUG_H_
#define _DEBUG_H_ 1

#include <kprintf.h>
#include <timer.h>

#define DEBUGP(str) (kprintf(__FILE__ ":%d <%s>:\n   >> ", __LINE__, __func__)?(kprintf(str),kwait(0,100000),1):0)
#define DEBUGF(...) (kprintf(__FILE__ ":%d <%s>:\n   >> ", __LINE__, __func__)?(kprintf(__VA_ARGS__),kwait(0,100000),1):0)

#endif
