#ifdef _HALUTAAN_TURHA_OTSIKKO
#ifndef _PFS_H
#define _PFS_H 1
#include <filesystem.h>

struct pfs_fs {
	struct fs base;
	/* Omat jutut */
	FILE *laite; // Luultavasti ainakin tämä
}

struct pfs_file {
	/* Omat jutut; tämä on funktiolle FILE */
	struct pfs_fs *fs; // Luultavasti ainakin tämä
}

extern fs_mount_t pfs_mount;

// Huom, yksikään ei ole extern
struct pfs_fs *pfs_mount_(FILE *device, unsigned int mode);
int pfs_umount(struct pfs_fs *this);

void *pfs_fopen(const char * restrict filename, const char * restrict mode);
int pfs_fclose(struct pfs_fileinfo *stream);

size_t pfs_fread(void *buf, size_t size, size_t count, void *stream);
size_t pfs_fwrite(void *buf, size_t size, size_t count, void *stream);

int pfs_fgetpos(struct pfs_fileinfo *stream, fpos_t *pos);
int pfs_fsetpos(struct pfs_fileinfo *stream, const fpos_t *pos);

#endif
#endif
