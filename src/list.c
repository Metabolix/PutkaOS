#include <list.h>
#include <stddef.h>
#include <memory/kmalloc.h>
#include <string.h>
#include <debug.h>

int _list_destroy_func(list_of_void *l)
{
	while (l->begin != l->end) {
		void *f = l->begin;
		l->begin = l->begin->next;
		--l->size;
		kfree(f);
	}
	if (l->size) {
		DEBUGF("Listalla on kokoa %d..!\n", l->size);
	}
	l->end->next = l->end->prev = 0;
	return 0;
}
list_iter_of_void _list_erase_func(list_iter_of_void l)
{
	void *to_free;
	if (!(to_free = l) || !l->list || (l == l->list->end)) return 0;
	if (l->list->begin == l) {
		l->list->begin = l->next;
	}
	if (l->next) l->next->prev = l->prev;
	if (l->prev) l->prev->next = l->next;
	l = l->next;
	--l->list->size;
	kfree(to_free);
	return l;
}

int _list_insert_func(list_iter_of_void l, ptrdiff_t pos, size_t size, const void *val, int is_after)
{
	struct list_member_of_void *u;
	u = kmalloc(pos + size);
	if (!u) {
		return -1;
	}
	u->list = l->list;
	l->list->size++;

	memcpy((char*)u + pos, val, size);
	if (is_after) {
		if ((u->next = l->next)) {
			l->next->prev = u;
		} else {
			DEBUGP("l->list->end->prev = u;");
			l->list->end->prev = u;
		}
		u->prev = l;
		l->next = u;
	} else {
		if ((u->prev = l->prev)) {
			l->prev->next = u;
		} else {
			l->list->begin = u;
		}
		u->next = l;
		l->prev = u;
	}
	return 0;
}

