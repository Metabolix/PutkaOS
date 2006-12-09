#ifndef _IRQ_H
#define _IRQ_H
void install_irq_handler(int irq, void (*irqhandler()));
void uninstall_irq_handler(int irq);
extern void irq_install();
extern void wait_irq(int irq);
extern void prepare_wait_irq(int irq);
#endif
