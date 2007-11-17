#include <exec.h>
#include <memory/kmalloc.h>
#include <multitasking/multitasking.h>

int bin_read(struct exec_file * f)
{
	f->code_start = kmalloc(f->size);
	f->code_size = f->size;
	fread(f->code_start, f->size, 1, f->file);
	f->entry_offset = 0;
	return 0;
}

struct binary_handler bhandlers[] = {
	{ bin_read }, 
	{ 0 }
};

int search_handler(struct exec_file * f)
{
	struct binary_handler * handler = bhandlers;
	while(handler) {
		if(!handler->read(f))
			return 0;
	}
	return -1;
}

int exec(const char * filename)
{
	struct exec_file * f;
	FILE * file;
	int ret;

	file = fopen(filename, "r");
	if(!file) { 
		return -2;
	}
	f = kcalloc(1, sizeof(struct exec_file));
	f->file = file;
	#ifdef READ_EXEC_HEADER_BYTES
	f->buf = kmalloc(EXEC_FILE_BYTES);
	fread(f->buf, EXEC_FILE_BYTES, 1, file);
	#endif
	fseek(file, 0, SEEK_END);
	f->size = ftell(file);
	fseek(file, 0, SEEK_SET);

	ret = search_handler(f);
	if(!ret) {
		new_process(f->code_start, f->code_size, f->entry_offset, f->stack, f->stack_size, 1);
	}

	#ifdef READ_EXEC_HEADER_BYTES
	kfree(f->buf);
	#endif
	kfree(f);
	fclose(file);
		
	return ret;
}

