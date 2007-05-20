#include <devices/specialdevs.h>
#include <devices/devmanager.h>
#include <filesys/file.h>
#include <filesys/filesystem.h>
#include <malloc.h>
#include <string.h>

int special_fclose(FILE *stream)
{
	kfree(stream);
	return 0;
}
int special_fflush(FILE *stream)
{
	return 0;
}
int special_fgetpos(FILE *stream, fpos_t *pos)
{
	*pos = stream->pos;
	return 0;
}
int special_fsetpos(FILE *stream, const fpos_t *pos)
{
	stream->pos = *pos;
	return 0;
}

size_t null_fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	stream->pos += size * count;
	return count;
}

size_t zero_fread(void *buf, size_t size, size_t count, FILE *stream)
{
	memset(buf, 0, size * count);
	stream->pos += size * count;
	return count;
}

size_t urandom_fread(void *buf, size_t size, size_t count, FILE *stream)
{
	static uint32_t a, b;
	size_t rsize = size * count;
	size_t size4 = rsize >> 2;
	uint32_t *buf32 = (uint32_t *) buf;
	while (size4--) {
		a -= 0x012357bfL; /* 16-bit alkuluku */
		b += 0xfedca201L; /* 16-bit alkuluku */
		a += (a >> 16) ^ (b << 16);
		b -= (a << 16) ^ (b >> 16);
		*buf32 = a ^ b;
		++buf32;
	}
	if (rsize & 3) {
		buf32 = (uint32_t*)((char*)buf + rsize - 4);
		a -= 0x012357bfL; /* 16-bit alkuluku */
		b += 0xfedca201L; /* 16-bit alkuluku */
		a += (a >> 16) ^ (b << 16);
		b -= (a << 16) ^ (b >> 16);
		*buf32 = a ^ b;
	}
	stream->pos += rsize;
	return count;
}

FILE *special_devopen(void)
{
	FILE * ret = kcalloc(sizeof(FILE) + sizeof(struct filefunc), 1);
	ret->func = (struct filefunc *) (ret + 1);
	ret->func->fflush = special_fflush;
	ret->func->fclose = special_fclose;
	ret->func->fgetpos = special_fgetpos;
	ret->func->fsetpos = special_fsetpos;
	return ret;
}

#define SPECIAL_OPENFUNC_TEMPLATE( stuff ) \
{ \
	FILE * ret = special_devopen(); \
	if (!ret) return 0; \
	{ stuff } \
	return ret; \
}

SPECIAL_OPENFUNC( null ) SPECIAL_OPENFUNC_TEMPLATE
(
	ret->func->fwrite = null_fwrite;
)

SPECIAL_OPENFUNC( zero ) SPECIAL_OPENFUNC_TEMPLATE
(
	ret->func->fread = zero_fread;
)

SPECIAL_OPENFUNC( urandom ) SPECIAL_OPENFUNC_TEMPLATE
(
	ret->func->fread = urandom_fread;
)
