#ifndef _MISC_ASM_H
#define _MISC_ASM_H 1

extern void asm_sti();
extern void asm_cli();
extern void asm_hlt();
extern void asm_nop();
extern void asm_ud0();
extern void asm_hlt_until_true(const char*);

extern void * asm_get_cr0(void);
extern void * asm_get_cr2(void);
extern void * asm_get_cr3(void);

extern void asm_set_cr0(void *);
extern void asm_set_cr3(void *);

extern void asm_idt_load(void *idt);

#endif
