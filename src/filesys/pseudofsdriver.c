#include <filesys/pseudofsdriver.h>
#include <malloc.h>

/* Esimerkiksi */
struct pfs_fs pfs = {
	{
		(fs_mount_t)  pfs_mount,
		(fs_umount_t) pfs_umount,
		{
			(fopen_t)     pfs_fopen,
			(fclose_t)    pfs_fclose,
			(fread_t)     pfs_fread,
			(fwrite_t)    pfs_fwrite,
			(fflush_t)    pfs_fflush,
			(ftell_t)     pfs_ftell,
			(fseek_t)     pfs_fseek,
			(fgetpos_t)   pfs_fgetpos,
			(fsetpos_t)   pfs_fsetpos
		},
		{
			(dmake_t)     pfs_dmake,
			(dopen_t)     pfs_dopen,
			(dclose_t)    pfs_dclose,
			(dread_t)     pfs_dread
		}
	},
	0
};

struct fs *pfs_mount(FILE *device, unsigned int mode)
{
	return 0; /* Virhe ;) */

	struct pfs_fs *retval = kcalloc(1, sizeof(struct pfs_fs));
	*retval = pfs;
	/* Ja tälle mountille spesifiset jutut... */
	return (struct fs *) retval;
}

int pfs_umount(struct pfs_fs *this)
{
	return -1; /* Virhe ;) */

	kfree(this);
	return 0;
}

void *pfs_fopen(struct pfs_fs *this, const char * filename, unsigned int mode)
{
	return 0; /* Virhe ;) */

	struct pfs_file *retval = kcalloc(1, sizeof(struct pfs_file));
	retval->std.func = &this->std.filefunc; /* = this, in fact */
	/* Ja mitä nyt tälle tiedostolle haluaakin tehdä... */
	return retval;
}

int pfs_fclose(struct pfs_file *stream)
{
	return -1; /* Virhe ;) */

	kfree(stream);
	return 0;
}

size_t pfs_fread(void *buf, size_t size, size_t count, struct pfs_file *stream)
{
	return 0;
}
size_t pfs_fwrite(void *buf, size_t size, size_t count, struct pfs_file *stream)
{
	return 0;
}
int pfs_fflush(FILE *stream)
{
	return EOF;
}
long pfs_ftell(FILE *stream)
{
	return 0;
}
int pfs_fseek(FILE *stream, long int offset, int origin)
{
	return -1;
}
int pfs_fgetpos(struct pfs_file *stream, fpos_t *pos)
{
	pos->lo_dword = pos->hi_dword = 0;
	return 0;
}
int pfs_fsetpos(struct pfs_file *stream, const fpos_t *pos)
{
	return EOF;
}

int pfs_dmake(struct fs *this, const char * dirname, uint_t owner, uint_t rights)
{
	return -1;
}

DIR *pfs_dopen(struct fs *this, const char * dirname)
{
	return 0;
}

int pfs_dclose(DIR *listing)
{
	return -1;
}

int pfs_dread(DIR *listing)
{
	return -1;
}
