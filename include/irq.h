#ifndef _IRQ_H
#define _IRQ_H 1

typedef void (*irq_handler_t)();

extern void install_irq_handler(unsigned int irq, irq_handler_t handler);
extern void uninstall_irq_handler(unsigned int irq);

extern void irq_install(void);
extern void wait_irq(unsigned int irq);
extern void prepare_wait_irq(unsigned int irq);

extern int is_in_irq_handler(void);

#endif
