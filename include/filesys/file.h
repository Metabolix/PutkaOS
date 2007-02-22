#ifndef _FILE_H
#define _FILE_H 1

#include <stddef.h>
#include <stdint.h>

typedef struct _fpos_t {
	size_t lo_dword, hi_dword;
} fpos_t;

typedef struct _FILE {
	fpos_t pos;
	short errno, eof;
	struct filefunc *func;
} FILE;

//#define _IOFBF 1
//#define _IOLBF 2
//#define _IONBF 3
//#define BUFSIZ 0x1000
#define EOF (-1)
//#define FOPEN_MAX 0x100
//#define FILENAME_MAX 0x400
//#define L_tmpnam 0x100
//#define TMP_MAX 0x100

#define SEEK_CUR 0
#define SEEK_SET 1
#define SEEK_END 2

//#define stdin  stdin
//#define stdout stdout
//#define stderr stderr

extern FILE *fopen(const char * filename, const char * mode);
extern int fclose(FILE *stream);
extern size_t fread(void *buf, size_t size, size_t count, FILE *file);
extern size_t fwrite(const void *buf, size_t size, size_t count, FILE *file);
extern int fflush(FILE *stream);
extern long ftell(FILE *stream);
extern int fseek(FILE *stream, long int offset, int origin);
extern int fgetpos(FILE *stream, fpos_t *pos);
extern int fsetpos(FILE *stream, const fpos_t *pos);
/*
extern int remove(const char *filename);
extern int rename(const char *old, const char *new);
extern FILE *tmpfile(void); // wb+
extern char *tmpnam(char *); // wb+
extern int fclose(FILE *file); // 0, EOF
extern int fflush(FILE *file); // 0, EOF
extern FILE *fopen(const char * filename, const char * mode);
extern FILE *freopen(const char * filename, const char * mode, FILE * stream);
*/

#endif
