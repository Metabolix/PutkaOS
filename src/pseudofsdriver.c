#include <pseudofsdriver.h>
#include <malloc.h>

extern fs_mount_t pfs_mount = (fs_mount_t) pfs_mount_;

struct pfs_fs *pfs_mount_(FILE *device, unsigned int mode)
{
	struct pfs_fs *retval = malloc(sizeof(struct pfs_fs));
	retval->fs_umount = pfs_umount;
	retval->fread = pfs_fread;
	retval->fwrite = pfs_fwrite;
	retval->fopen = pfs_fopen;
	retval->fclose = pfs_fclose;
	retval->fgetpos = pfs_fgetpos;
	retval->fsetpos = pfs_fsetpos;
	return retval;
}
int pfs_umount(struct pfs_fs *this)
{
	free(this);
}

void *pfs_fopen(const char * restrict filename, const char * restrict mode)
{
	struct pfs_file *retval = malloc(sizeof(struct pfs_file));
}

int pfs_fclose(struct pfs_file *stream)
{
	free(stream);
	return 0;
}

size_t pfs_fread(void *buf, size_t size, size_t count, void *stream)
{
	return 0;
}
size_t pfs_fwrite(void *buf, size_t size, size_t count, void *stream)
{
	return 0;
}
int pfs_fgetpos(struct pfs_fileinfo *stream, fpos_t *pos)
{
	pos->lo_dword = pos->hi_dword = 0;
	return 0;
}
int pfs_fsetpos(struct pfs_fileinfo *stream, const fpos_t *pos)
{
	return EOF;
}
