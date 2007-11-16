#ifndef _SYS_FILE_H
#define _SYS_FILE_H 1

enum FILE_MODE_FLAGS {
	FILE_MODE_READ = 1,
	FILE_MODE_WRITE = 2,
	FILE_MODE_RW = FILE_MODE_READ | FILE_MODE_WRITE,
	FILE_MODE_APPEND = 4,
	FILE_MODE_CLEAR = 8,
	FILE_MODE_DONT_CREATE = 16,
	FILE_MODE_MUST_BE_NEW = 32,
};

enum IOCTL_VALUES {
        IOCTL_STD_BEGIN = 0,
        IOCTL_TRUNCATE_FILE = 1,

        IOCTL_STD_END
};

extern FILE *fopen2(const char * filename, uint_t flags);
extern int ioctl(FILE * stream, int request, intptr_t param);

#endif
