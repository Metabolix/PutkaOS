#ifndef _PUTKAOS_H
#define _PUTKAOS_H
#include <irq.h>
#include <thread.h>

#define threading_on() (!in_irq_handler() && active_thread_ptr && active_process_ptr) /* Eh? */

#endif

