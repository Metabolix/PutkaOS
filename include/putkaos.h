#ifndef _PUTKAOS_H
#define _PUTKAOS_H
#include <irq.h>
#include <thread.h>

#define sti() asm("sti")
#define cli() asm("cli")
#define threading_on() (!in_irq_handler() && active_thread != NO_THREAD) /* Make better thing to replace that active_thread != NO_THREAD */

#endif

