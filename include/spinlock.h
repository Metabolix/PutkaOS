#ifndef _SPINLOCK_H
#define _SPINLOCK_H

struct spinlock {
	unsigned char count;
};

extern void spinl_init(struct spinlock * spinl);
extern void spinl_spin(struct spinlock * spinl);
extern void spinl_lock(struct spinlock * spinl);
extern void spinl_unlock(struct spinlock * spinl);
extern char spinl_locked(struct spinlock * spinl);

#endif
