#ifndef _THREAD_H
#define _THREAD_H 1

/**
 * struct thread - säikeen tiedot
 * @esp: esp, jotta iret saadaan tehtyä
 * @ss: stack segment, jotta iret saadaan tehtyä
 * @pid: omistava prosessi
 * @state: tilanne (vapaa, käynnissä, päättynyt, ...)
 * @thread_of_proc: monesko prosessin säikeistä tämä on (= pinon numero)
 *
 * Badly unready
**/
struct thread {
	struct regs *esp;
	uint16_t ss, reserved_01;

	uint_t uid, gid;
	uint_t state;
	pid_t pid;

	int thread_of_proc;
};

extern struct thread threads[MAX_THREADS];
extern size_t thread_count;

extern void clean_threads(void);
extern tid_t find_running_thread(void);

#endif
