#ifndef _DEVMANAGER_H
#define _DEVMANAGER_H 1

#include <filesystem.h>

/* Moi, Megant 8) Tee loppuun vain. Voisi tehdä ihan filesystemiksi asti, kaikki fopenit ja muut... */

struct DEVICE {
	FILE file;
};

extern FILE *dev_fopen(const char * restrict filename, const char * restrict mode);

extern int dev_fclose(FILE *stream);

#endif
