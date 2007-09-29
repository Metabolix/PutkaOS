#ifndef _LIST_H
#define _LIST_H 1

#include <stddef.h>
#include <string.h>

struct list_of_void;
struct list_member_of_void;
#define list_ender_of_void list_member_of_void
typedef struct list_of_void list_of_void;
typedef struct list_member_of_void * list_iter_of_void;

struct list_member_of_void {
	struct list_of_void *list;
	struct list_member_of_void *next, *prev;
};
struct list_of_void {
	size_t size;
	struct list_member_of_void *begin, *end;
	struct list_ender_of_void real_end;
};

#define LIST_TYPE(nimi, tyyppi) \
/* BEGIN OF LIST */ \
struct list_of_ ## nimi ; \
struct list_member_of_ ## nimi ; \
typedef struct list_of_ ## nimi list_of_ ## nimi ; \
typedef struct list_member_of_ ## nimi * list_iter_of_ ## nimi ; \
\
struct list_ender_of_ ## nimi { \
        struct list_of_ ## nimi *list; \
	struct list_member_of_ ## nimi *next, *prev; \
}; \
struct list_member_of_ ## nimi { \
        struct list_of_ ## nimi *list; \
	struct list_member_of_ ## nimi *next, *prev; \
	tyyppi tavara; \
}; \
\
struct list_of_ ## nimi { \
	size_t size; \
	struct list_member_of_ ## nimi *begin, *end; \
	struct list_ender_of_ ## nimi real_end; \
} /* Huomaa puolipisteen puuttuminen */ \
/* END OF LIST */

#define list_init(list_a) {\
	memset(&(list_a), 0, sizeof((list_a))); \
	(list_a).begin = (list_a).end = (void*)&(list_a).real_end; \
	(list_a).real_end.list = &(list_a);}
#define list_destroy(list) _list_destroy_func((list_of_void*)&list)

#define list_begin(list) ((list).begin)
#define list_end(list) ((list).end)
#define list_size(list) ((const int)((list).size))

#define list_next(iter) ((iter) ? ((iter)->next) : 0)
#define list_prev(iter) ((iter) ? ((iter)->prev) : 0)

#define list_inc(iter) ((iter) = list_next(iter))
#define list_dec(iter) ((iter) = list_prev(iter))

#define list_item(iter) ((iter)->tavara)

#define list_loop(iter, list) for (iter = list_begin(list); iter != list_end(list); list_inc(iter))

#define list_insert_after(iter, val) \
	((!(iter) || !(iter)->list || !(iter)->next || (sizeof((iter)->tavara) != sizeof(val))) ? -1 : \
	_list_insert_func( \
	(list_iter_of_void) iter, \
	(char*)&((iter)->tavara) - (char*)(iter), \
	sizeof(val), &val, 1))

#define list_insert(iter, val) list_insert_before(iter, val)
#define list_insert_before(iter, val) \
	((!(iter) || !(iter)->list || (sizeof((iter)->tavara) != sizeof(val))) ? -1 : \
	_list_insert_func( \
	(list_iter_of_void) iter, \
	(char*)&((iter)->tavara) - (char*)(iter), \
	sizeof(val), &val, 0))

#define list_erase(iter) _list_erase_func((list_iter_of_void)iter)

// Älä koske näihin, prkl!
extern list_iter_of_void _list_erase_func(list_iter_of_void l);
extern int _list_destroy_func(list_of_void *l);
extern int _list_insert_func(list_iter_of_void l, ptrdiff_t pos, size_t size, const void *val, int is_after);

#if 0
// Esimerkki

struct koepiste {
	int x, y;
};

LIST_TYPE(piste, struct koepiste);

list_of_piste pisteet;
list_iter_of_piste p;

struct koepiste kp;

void printtaa_lista()
{
	list_iter_of_piste i;
	i = list_begin(pisteet);
	kprintf("size: %d\n", list_size(pisteet));
	while (i != list_end(pisteet)) {
		kprintf("%p: (%d, %d)\n", i, list_item(i).x, list_item(i).y);
		list_inc(i); // i = list_next(i);
	}
	print("\n");
}

void testattava_koodi()
{
	list_init(pisteet);

	kp.x = 1; kp.y = 0;
	list_insert(list_begin(pisteet), kp);
	kp.x = 2; kp.y = 1;
	list_insert(list_begin(pisteet), kp);
	kp.x = 4; kp.y = 2;
	list_insert(list_begin(pisteet), kp);
	printtaa_lista();

	p = list_begin(pisteet); // 0.
	list_inc(p); // 1.
	list_inc(p); // 2.
	kp.x = 8; kp.y = 3;
	list_insert(p, kp); // Tulee ennen tätä (nyk. 2.), siis uusi 2.
	kp.x = 16; kp.y = 4;
	list_insert_after(p, kp); // Tulee tämän jälkeen (nyk. 3.), siis uusi 4.
	kp.x = 32; kp.y = 5;
	list_insert(list_end(pisteet), kp); // Listan loppuun
	printtaa_lista();

	p = list_begin(pisteet); // 0.
	list_inc(p); // 1.
	list_inc(p); // 2.
	list_erase(p); // Poistetaan se
	printtaa_lista();

	list_destroy(pisteet);
	printtaa_lista();
}
#endif


#endif
