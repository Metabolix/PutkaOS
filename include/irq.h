#ifndef _IRQ_H
#define _IRQ_H 1

typedef void (*irq_handler_t)(void);

extern void install_irq_handler(unsigned int irq, irq_handler_t handler);
extern void uninstall_irq_handler(unsigned int irq);

extern void irq_install(void);
extern void irq_unmask(void);
extern void wait_irq(unsigned int irq);
extern void prepare_wait_irq(unsigned int irq);

extern int is_in_irq_handler(void);

#endif
