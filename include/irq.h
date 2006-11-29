#ifndef _IRQ_H
#define _IRQ_H
void install_irq_handler(int irq, void (*irqhandler()));
void uninstall_irq_handler(int irq);
void install_irq();
void wait_irq(int irq);
#endif
