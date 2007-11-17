#ifndef _EXEC_H
#define _EXEC_H 1

#include <stdlib.h>
#include <stdio.h>

#define EXEC_FILE_BYTES 128 /* How many first bytes we'll get */
#undef READ_EXEC_HEADER_BYTES
struct exec_file {
	FILE * file;

	char * buf; /* For first bytes */
	uint32_t size;
	uint32_t code_size;
	uint32_t stack_size;
	uint32_t entry_offset;

	void * code_start;
	void * stack;
};

typedef int (*exec_read_t)(struct exec_file * file);

struct binary_handler {
	exec_read_t read;
};

extern int exec(const char * filename);
#endif

