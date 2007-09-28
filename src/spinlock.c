#include <spinlock.h>
#include <thread.h>
#include <misc_asm.h>

void spinl_init(struct spinlock * spinl) {
	spinl->count = 0;
}

void spinl_unlock(struct spinlock * spinl) {
        spinl->count = 0;
}

void spinl_spin(struct spinlock * spinl) {
	while (spinl->count) asm_nop(spinl);
}

void spinl_lock(struct spinlock * spinl) {
	for(;;) { /* Until we get the lock */
		spinl_spin(spinl); /* Someone could catch the lock still after this... */
		asm_cli(); /* I think that we need these here, because 2 threads could try to lock on same time think that they (both) succeeded (It is possible?) */
		if (!spinl_locked(spinl)) {
			spinl->count = 1;
			asm_sti();
			break;
		}
		asm_sti();
	}
}

int spinl_locked(struct spinlock * spinl) {
	return spinl->count;
}
