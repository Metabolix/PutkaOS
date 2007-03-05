#ifndef _PFS_H
#define _PFS_H 1
#include <filesys/filesystem.h>

struct pfs_fs {
	/* Yleiset */
	struct fs std;

	/* Omat jutut */
	FILE *laite; // Luultavasti ainakin tämä
};

struct pfs_file {
	/* Yleisesti tiedosto */
	FILE std;

	/* Omat jutut; tämä on funktiolle FILE */
};

struct pfs_dir {
	/* Yleisesti tiedosto */
	DIR std;

	/* Omat jutut; tämä on funktiolle FILE */
};

extern struct fs *pfs_mount(FILE *device, uint_t mode);

// Huom, yksikään ei ole extern
int pfs_umount(struct pfs_fs *this);

void *pfs_fopen(struct pfs_fs *this, const char * filename, uint_t mode);
int pfs_fclose(struct pfs_file *stream);

size_t pfs_fread(void *buf, size_t size, size_t count, struct pfs_file *stream);
size_t pfs_fwrite(void *buf, size_t size, size_t count, struct pfs_file *stream);

int pfs_fflush(FILE *stream);

int pfs_fgetpos(struct pfs_file *stream, fpos_t *pos);
int pfs_fsetpos(struct pfs_file *stream, const fpos_t *pos);

int pfs_dmake(struct pfs_fs *this, const char * dirname, uint_t owner, uint_t rights);
struct pfs_dir *pfs_dopen(struct pfs_fs *this, const char * dirname);
int pfs_dread(struct pfs_dir *listing);
int pfs_dclose(struct pfs_dir *listing);

#endif
