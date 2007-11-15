#include <filesys/file.h>
#include <filesys/mount.h>
#include <string.h>
#include <stdint.h>

struct filefunc nil_filefunc = {0};

static FILE *sys_fopen2(const char * filename, uint_t flags)
{
	const struct mount *mnt;

	if (filename)
	if (flags & (FILE_MODE_READ | FILE_MODE_WRITE))
	if ((mnt = mount_etsi_kohta(&filename)))
	if (mnt->fs)
	if ((mnt->fs->mode & FILE_MODE_WRITE) >= (flags & FILE_MODE_WRITE))
	if (mnt->fs->filefunc.fopen) {
		FILE *f = mnt->fs->filefunc.fopen(mnt->fs, filename, flags);
		if (!f) return f;
		if (!f->func) f->func = &nil_filefunc;
		if (f->mode == 0) f->mode = flags;
		return f;
	}
	return 0;
}

static int sys_fclose(FILE *stream)
{
	if (stream && stream->func->fclose) {
		return stream->func->fclose(stream);
	}
	return -1;
}

static size_t sys_fread(void *buf, size_t size, size_t count, FILE *stream)
{
	if (!buf || !stream || !(stream->mode & FILE_MODE_READ)) return 0;
	if (size == 0 || count == 0) return 0;
	if (stream->func->fread) {
		return stream->func->fread(buf, size, count, stream);
	}
	return 0;
}

static size_t sys_fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	if (!buf || !stream || !(stream->mode & FILE_MODE_WRITE)) return 0;
	if (size == 0 || count == 0) return 0;
	if (stream->mode & FILE_MODE_APPEND) {
		if (fsetpos(stream, &stream->size)) {
			return 0;
		}
	}
	if (stream->func->fwrite) {
		return stream->func->fwrite(buf, size, count, stream);
	}
	return 0;
}

static int sys_fgetpos(FILE *stream, fpos_t *pos)
{
	if (stream) {
		*pos = stream->pos;
		return 0;
	}
	memset(pos, 0, sizeof(fpos_t));
	return EOF;
}

static int sys_fsetpos(FILE *stream, const fpos_t *pos)
{
	if (stream && stream->func->fsetpos) {
		return stream->func->fsetpos(stream, pos);
	}
	return EOF;
}

static int sys_fflush(FILE *stream)
{
	if (!stream) {
		return EOF;
	}
	if (!(stream->mode & FILE_MODE_WRITE)) {
		return 0;
	}
	if (!stream->func || !stream->func->fflush) {
		return EOF;
	}
	return stream->func->fflush(stream);
}

static int sys_fseek(FILE *stream, long int offset, int origin)
{
	fpos_t pos;
	switch (origin) {
		case SEEK_SET:
			pos = offset;
			break;
		case SEEK_END:
			pos = stream->size - offset;
			break;
		case SEEK_CUR:
			pos = stream->pos + offset;
			break;
		default:
			return EOF;
	}
	return (fsetpos(stream, &pos) ? EOF : 0);
}

static int sys_ioctl(FILE * stream, int request, intptr_t param)
{
	if (stream && stream->func && stream->func->ioctl) {
		return stream->func->ioctl(stream, request, param);
	}
	// TODO: error code for the FS not supporting this?
	return -1; // TODO: errno EBADF
}

#define SYSCALL_TYPEDEFS 1
#include <sys/syscalls.h>

/**
* syscall_fopen2: fopen2(ecx, edx)
**/
FILE * syscall_fopen2(const char *name, uint_t flags)
{
	return sys_fopen2(name, flags);
}

/**
* syscall_fclose: fclose(ebx);
**/
int syscall_fclose(FILE *f)
{
	return sys_fclose(f);
}

/**
* syscall_fread: fread(ecx[0], ecx[1], ecx[2], ecx[3]);
**/
size_t syscall_fread(fread_params_t *ecx)
{
	return sys_fread(ecx->buf, ecx->size, ecx->count, ecx->f);
}

/**
* syscall_fwrite: fwrite(ecx[0], ecx[1], ecx[2], ecx[3]);
**/
size_t syscall_fwrite(fwrite_params_t *ecx)
{
	return sys_fwrite(ecx->buf, ecx->size, ecx->count, ecx->f);
}

/**
* syscall_fflush: fflush(ecx);
**/
int syscall_fflush(FILE * restrict stream)
{
	return sys_fflush(stream);
}

/**
* syscall_fseek: fseek(ecx[0], ecx[1], ecx[2]);
**/
int syscall_fseek(fseek_params_t *params)
{
	return sys_fseek(params->f, params->offset, params->origin);
}

/**
* syscall_fgetpos: fgetpos(ecx, edx);
**/
int syscall_fgetpos(FILE * restrict stream, fpos_t * restrict pos)
{
	return sys_fgetpos(stream, pos);
}

/**
* syscall_fsetpos: fsetpos(ecx, edx);
**/
int syscall_fsetpos(FILE * restrict stream, const fpos_t * restrict pos)
{
	return sys_fsetpos(stream, pos);
}

/**
* syscall_ioctl: ioctl(ecx[0], ecx[1], ecx[2]);
**/
int syscall_ioctl(ioctl_params_t *params)
{
	return sys_ioctl(params->f, params->request, params->param);
}
