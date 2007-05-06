#ifndef _PUTKAOS_H
#define _PUTKAOS_H
#include <irq.h>
#include <thread.h>

extern void asm_sti(void);
extern void asm_cli(void);
extern void asm_hlt(void);
extern void asm_nop(void);
extern void asm_hlt_until_true(const char*);
#define threading_on() (!in_irq_handler() && active_thread_ptr && active_process_ptr) /* Eh? */

#endif

