#ifndef _LIST_H
#define _LIST_H

/**
* Kahteen suuntaan linkitetty lista
* TODO: Virheenkäsittely, debuggaus, enkkukäännös(?)
*
* Käyttöesimerkki lopussa
**/



typedef struct { void* prev; void* next; void* data; } member;
typedef struct { member* first; member* last; size_t num; } list;

#define LIST list*
/* Init: Varataan muistia listastruktille */
#define LIST_INIT(x) x = kmalloc(sizeof(list)); x->num=0; x->first=0; x->last=0;
#define LIST_NUM_OF_ITEMS(x) x->num;
#define LIST_ITERATOR(x) member*
/* Iteraattori listan alkuun, jos lista on tyhjä i=0 */
#define LIST_START(x, i) i=x->first;

/* Iteraattori listan loppuun */
#define LIST_END(x, i) i=x->last;


/* lista, iteraattori, data, tyyppi */
/* nollaiteraattori asetetaan tyhjän listan tapauksessa osoittamaan alkuun */
#define LIST_ADD_BEFORE(l, i, x, type) \
	if(i) {	/* helppo tapaus */ \
		member* mem = kmalloc(sizeof(member)); \
		type* dat = kmalloc(sizeof(type)); *dat=x; mem->data=dat; \
		mem->next=i; mem->prev=i->prev; \
		if (i->prev) { 	member* tmp = (member*)i->prev; tmp->next=i; } \
		i->prev=mem; \
		if (l->first==i) l->first=mem; \
		l->num++; \
	} \
	else { /* i=0, jos lista tyhjä niin listan alkuun, muutoin ei tehdä mitään */ \
		if (l->first==0) { \
			member* mem = kmalloc(sizeof(member)); \
			type* dat = kmalloc(sizeof(type)); *dat=x; mem->data=dat; \
			l->first=mem; l->last=mem; l->num=1; i=mem;\
		} \
		else { /* lista ei ole tyhjä mutta iteraattori osoittaa nollaa, ei näin */ \
			kprintf("Lista epätyhjä mutta iteraattori 0, en osaa\n"); \
		} \
	}

/* lista, iteraattori, data, tyyppi */
/* nollaiteraattori asetetaan tyhjän listan tapauksessa osoittamaan alkuun */
#define LIST_ADD_AFTER(l, i, x, type) \
	if(i) {	/* helppo tapaus */ \
		member* mem = kmalloc(sizeof(member)); \
		type* dat = kmalloc(sizeof(type)); *dat=x; mem->data=dat; \
		mem->next=i->next; mem->prev=i; \
		if (i->next) {member* tmp = (member*)i->next; tmp->prev=mem; }\
		i->next=mem; \
		if (l->last == i) l->last=mem; \
		l->num++; \
	} \
	else { /* i=0, jos lista tyhjä niin listan alkuun, muutoin ei tehdä mitään */ \
		if (l->first==0 || l->num==0) { \
			member* mem = kmalloc(sizeof(member)); \
			type* dat = kmalloc(sizeof(type)); *dat=x; mem->data=dat; \
			l->first=mem; l->last=mem; l->num=1; i=mem; mem->next=0; mem->prev=0;\
		} \
		else { /* lista ei ole tyhjä mutta iteraattori osoittaa nollaa, ei näin */ \
			kprintf("Lista epätyhjä mutta iteraattori 0, en osaa\n"); \
		} \
	}

/* Poista alkio i listasta l */
#define LIST_DELETE(l,i) \
	if (i) { \
		free(i->data); \
		member* tmp = i->prev; \
		if (tmp) tmp->next=i->next; \
		tmp=i->next; \
		if (tmp) tmp->prev=i->prev; \
		if (l->first==i) l->first=i->next; \
		if (l->last==i) l->last=i->prev; \
		free(i); l->num--; \
	} else kprintf("Yritettiin poistaa nollaiteraattori\n");


#define LIST_NEXT(i) if (i) { if (i->next) i=(member*)i->next; }
#define LIST_PREVIOUS(i) if(i) { if(i->prev) i=i->prev; } 

/* TODO: Segfaulttaa kai jos i=0 */
#define LIST_MEMBER(i, type) *((type*)i->data)

/* Onko lista lopussa? tosi, jos iteraattori osoittaa viimeistä alkiota */
#define LIST_AT_END(l, i) (l->last==i)


/*
KÄYTTÖESIMERKKI

LIST lista;
LIST_INIT(lista);

LIST_ITERATOR(lista) i;
LIST_START(lista, i);

LIST_ADD_AFTER(lista, i, 2, int);
LIST_ADD_BEFORE(lista, i, -2, int);
LIST_ADD_AFTER(lista, i, 1000, int);
LIST_ADD_AFTER(lista, i, 2000, int);
LIST_DELETE(lista,i);

LIST_START(lista, i);

int d; int items=LIST_NUM_OF_ITEMS(lista);
for (d=0; d<items; d++) {
	printf("itemi: %d\n", LIST_MEMBER(i, int));
	LIST_NEXT(i);
} 

if (LIST_AT_END(lista,i))
	printf("Joo kaikki (t)ulostettiin\n");


int num = LIST_NUM_OF_ITEMS(lista);
printf("itemejä %d\n", num);

Listassa on siis lopussa luvut -2, 2000 ja 1000
*/

#endif
