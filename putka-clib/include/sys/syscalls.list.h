/*
#define SYSCALL_MACRO(number, func, name, prototype, short_desc, long_desc)
*/

// SYSCALL_MACRO( 0, syscall_illegal, "Reserved / null", "", "" )

#ifndef SYSCALL_MACRO_TYPES
#define SYSCALL_MACRO_TYPES 1
#define SYSCALL_MACRO_111110(a,b,c,d,e) SYSCALL_MACRO(a,b,c,d,e,"")
#define SYSCALL_MACRO_111111(a,b,c,d,e,f) SYSCALL_MACRO(a,b,c,d,e,f)
#endif

#ifndef _SYSCALL_TYPEDEFS_H
#define _SYSCALL_TYPEDEFS_H 1

typedef intptr_t (*syscall_t)(intptr_t ecx, intptr_t edx);

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>

typedef struct {
	void *buf;
	size_t size;
	size_t count;
	FILE *f;
} fread_params_t;

typedef struct {
	const void *buf;
	size_t size;
	size_t count;
	FILE *f;
} fwrite_params_t;

typedef struct {
	FILE *f;
	long int offset;
	int origin;
} fseek_params_t;

typedef struct {
	FILE *f;
	int request;
	intptr_t param;
} ioctl_params_t;

#endif

#define SYSCALL_PRINT (0x01)
#define SYSCALL_MALLOC (0x02)
#define SYSCALL_FREE (0x03)
#define SYSCALL_REALLOC (0x04)

#define SYSCALL_GET_SYSTEM_TIME (0x08)
#define SYSCALL_GET_UPTIME (0x09)

#define SYSCALL_FOPEN2 (0x10)
#define SYSCALL_FCLOSE (0x11)
#define SYSCALL_FREAD (0x12)
#define SYSCALL_FWRITE (0x13)
#define SYSCALL_FGETPOS (0x14)
#define SYSCALL_FSETPOS (0x15)
#define SYSCALL_FSEEK (0x16)
#define SYSCALL_FFLUSH (0x17)
#define SYSCALL_IOCTL (0x18)

#ifdef SYSCALL_MACRO

SYSCALL_MACRO_111110(
	SYSCALL_PRINT,
	syscall_print,
	"print",
	void syscall_print(const char *text),
	"Tulostaa tekstin. Voimassa toistaiseksi."
)

/**
* Memory
* kernel => memory/malloc.c
* user => stdlib.h (TODO)
**/
SYSCALL_MACRO_111110(
	SYSCALL_MALLOC,
	syscall_malloc,
	"malloc",
	void *syscall_malloc(size_t size),
	"Varaa muistia"
)
SYSCALL_MACRO_111110(
	SYSCALL_FREE,
	syscall_free,
	"free",
	void syscall_free(void *ptr),
	"Vapauttaa muistia"
)
SYSCALL_MACRO_111110(
	SYSCALL_REALLOC,
	syscall_realloc,
	"realloc",
	void *syscall_realloc(void *ptr, size_t new_size),
	"Varaa uuden muistialueen ja kopioi vanhan muistin sinne"
)

/**
* Time
* kernel => timer.c
* user => sys/time.h
**/
SYSCALL_MACRO_111110(
	SYSCALL_GET_SYSTEM_TIME,
	syscall_get_system_time,
	"get_system_time",
	int syscall_get_system_time(struct tm *sys_time_ptr),
	"Hakee systeemin kellonajan"
)
SYSCALL_MACRO_111110(
	SYSCALL_GET_UPTIME,
	syscall_get_uptime,
	"get_uptime",
	int syscall_get_uptime(struct timeval *uptime_ptr),
	"Hakee uptimen"
)

/**
* Files
* kernel => filesys/file.c
* user => stdio.h, sys/file.h
**/
SYSCALL_MACRO_111110(
	SYSCALL_FOPEN2,
	syscall_fopen2,
	"fopen2",
	FILE * syscall_fopen2(const char *filename, uint_t flags),
	"Avaa tiedoston; mode-stringin sijaan int-lippuja"
)
SYSCALL_MACRO_111110(
	SYSCALL_FCLOSE,
	syscall_fclose,
	"fclose",
	int syscall_fclose(FILE *f),
	"Sulkee tiedoston"
)
SYSCALL_MACRO_111111(
	SYSCALL_FREAD,
	syscall_fread,
	"fread",
	size_t syscall_fread(fread_params_t *params),
	"Lukee tiedostosta puskuriin",
	"typedef struct {void *buf; size_t size; size_t count; FILE *f;} fread_params_t;"
)
SYSCALL_MACRO_111111(
	SYSCALL_FWRITE,
	syscall_fwrite,
	"fwrite",
	size_t syscall_fwrite(fwrite_params_t *params),
	"Kirjoittaa puskurista tiedostoon",
	"typedef struct {const void *buf; size_t size; size_t count; FILE *f;} fwrite_params_t;"
)
SYSCALL_MACRO_111110(
	SYSCALL_FFLUSH,
	syscall_fflush,
	"fflush",
	int syscall_fflush(FILE * stream),
	"Kirjoittaa tiedoston puskurin levylle"
)
SYSCALL_MACRO_111111(
	SYSCALL_FSEEK,
	syscall_fseek,
	"fseek",
	int syscall_fseek(fseek_params_t *params),
	"Siirtyy haluttuun kohti tiedostoa",
	"typedef struct {FILE *f; long int offset; int origin;} fseek_params_t;"
)
SYSCALL_MACRO_111110(
	SYSCALL_FGETPOS,
	syscall_fgetpos,
	"fgetpos",
	int syscall_fgetpos(FILE * stream, fpos_t * pos),
	"Kertoo sijainnin tiedostossa (fpos_t = uint64_t)"
)
SYSCALL_MACRO_111110(
	SYSCALL_FSETPOS,
	syscall_fsetpos,
	"fsetpos",
	int syscall_fsetpos(FILE * stream, const fpos_t * pos),
	"Asettaa sijainnin tiedostossa (fpos_t = uint64_t)"
)
SYSCALL_MACRO_111111(
	SYSCALL_IOCTL,
	syscall_ioctl,
	"ioctl",
	int syscall_ioctl(ioctl_params_t *params),
	"Asettaa mahdollisia parametreja tiedostolle tai laitteelle",
	"typedef struct {FILE *f; int request; intptr_t param;} ioctl_params_t;"
)

#endif
