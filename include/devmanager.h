#ifndef _DEVMANAGER_H
#define _DEVMANAGER_H 1

#include <filesystem.h>
#include <stdint.h>

/* Moi, Megant 8) Tee loppuun vain. Voisi tehdä ihan filesystemiksi asti, kaikki fopenit ja muut... */

struct DEVICE {
	FILE file;
};

extern struct fs devfs;

/*
 * Internals
 */
FILE *dev_fopen(struct fs *this, const char * filename, uint_t mode);
int dev_fclose(FILE *stream);

#endif
