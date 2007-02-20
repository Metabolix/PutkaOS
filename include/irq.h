#ifndef _IRQ_H
#define _IRQ_H

void install_irq_handler(unsigned int irq, void (*irqhandler)());
void uninstall_irq_handler(unsigned int irq);

extern void irq_install(void);
extern void wait_irq(unsigned int irq);
extern void prepare_wait_irq(unsigned int irq);
extern unsigned char in_irq_handler(void);

#endif
