#include <vt.h>
//#include <devices/devmanager.h>
#include <malloc.h>
#include <string.h>
#include <screen.h>
#include <list.h>
#include <filesys/filesystem.h>

struct stream_pair
{
	FILE *stream1, *stream2;
	char *to1buf, *to2buf;
	int buf1len, buf2len;
};

LIST_TYPE(stream_pairs, struct stream_pair);
list_of_stream_pairs stream_pair_list;

size_t vt_fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	if(count < 0 || size < 0) return 0;
	list_iter_of_stream_pairs iter;
	list_loop(iter, stream_pair_list) {
		struct stream_pair *p = &list_item(iter);
		if (p->stream1 == stream) {
			size_t oldlen = p->buf2len;
			size_t newlen = oldlen + size*count;
			if(p->buf2len==0){
				p->to2buf = (char*)kmalloc(size*count);
			}
			else p->to2buf = (char*)krealloc(p->to2buf, newlen);
			memcpy(p->to2buf + oldlen, buf, size*count);
			p->buf2len = newlen;
			return count;
		}
		if (p->stream2 == stream) {
			size_t oldlen = p->buf1len;
			size_t newlen = oldlen + size*count;
			if(p->buf1len==0){
				p->to1buf = (char*)kmalloc(size*count);
			}
			else p->to1buf = (char*)krealloc(p->to1buf, newlen);
			memcpy(p->to1buf + oldlen, buf, size*count);
			p->buf1len = newlen;
			return count;
		}
	}
	kprintf("vt_fwrite(): could not find stream from list\n");
	return 0;
}

size_t vt_fread(void *buf, size_t size, size_t count, FILE *stream)
{
	if(count < 0 || size < 0) return 0;
	list_iter_of_stream_pairs iter;
	list_loop(iter, stream_pair_list) {
		struct stream_pair *p = &list_item(iter);
		if (p->stream1 == stream) {
			size_t oldlen = p->buf1len;
			size_t readlen = size*count;
			size_t newlen = oldlen - readlen;
			if(oldlen < readlen){
				newlen = 0;
				readlen = oldlen;
			}
			memcpy(buf, p->to1buf, readlen);
			p->to1buf = (char*)krealloc(p->to1buf + readlen, newlen);
			return readlen;
		}
		if (p->stream2 == stream) {
			size_t oldlen = p->buf2len;
			size_t readlen = size*count;
			size_t newlen = oldlen - readlen;
			if(oldlen < readlen){
				newlen = 0;
				readlen = oldlen;
			}
			memcpy(buf, p->to2buf, readlen);
			p->to2buf = (char*)krealloc(p->to2buf + readlen, newlen);
			return readlen;
		}
	}
	kprintf("vt_fread(): could not find stream from list\n");
	return 0;
}

int vt_ioctl(FILE *stream, int request, uintptr_t param)
{
	list_iter_of_stream_pairs iter;
	list_loop(iter, stream_pair_list) {
		struct stream_pair *p = &list_item(iter);
		//FILE *this;
		FILE *other;
		char *tothisbuf;
		int tothisbuflen;
		if (p->stream1 == stream) {
			//this = p->stream1;
			other = p->stream2;
			tothisbuf = p->to1buf;
			tothisbuflen = p->buf1len;
		}
		else if (p->stream2 == stream) {
			//this = p->stream2;
			other = p->stream1;
			tothisbuf = p->to2buf;
			tothisbuflen = p->buf2len;
		}
		else continue;

		if(request == IOCTL_VT_SET_FWRITE){
			struct filefunc *func;
			func = (struct filefunc*)other->func;
			func->fwrite = (fwrite_t)param;
			if(func->fwrite==NULL){
				func->fwrite = (fwrite_t)vt_fwrite;
			}
			else{
				int a = func->fwrite(tothisbuf, sizeof(char), tothisbuflen,
						other);
				kfree(tothisbuf);
				if(a != tothisbuflen) return 1;
				else return 0;
			}
		}
		else{
			return 1;
		}
	}
	return 1;
}

int vt_fclose(FILE *stream)
{
	kprintf("vt_fclose()\n");
	list_iter_of_stream_pairs iter;
	list_loop(iter, stream_pair_list) {
		if (list_item(iter).stream1 == stream || list_item(iter).stream2 == stream) {
			kprintf("closing\n");
			list_item(iter).stream1 = NULL;
			list_item(iter).stream2 = NULL;
			kfree(list_item(iter).to2buf);
			kfree(list_item(iter).to1buf);
			list_erase(iter);
			return 0;
		}
	}
	return 1;
}

/*FILE * vt_getpair(FILE *stream)
{
	list_iter_of_stream_pairs iter;
	list_loop(iter, stream_pair_list) {
		if (list_item(iter).stream1 == stream) {
			return list_item(iter).stream2;
		}
		if (list_item(iter).stream2 == stream) {
			return list_item(iter).stream1;
		}
	}
	return NULL;
}*/

int vt_get(FILE **streams)
{
	kprintf("vt_get()\n");

	FILE *stream1 = (FILE*)kmalloc(sizeof(FILE));
	if(stream1 == NULL){
		kprintf("vt_open(): kmalloc error 1\n");
		return 1;
	}
	memset(stream1, 0, sizeof(struct filefunc));
	stream1->mode = FILE_MODE_READ + FILE_MODE_WRITE;

	FILE *stream2 = (FILE*)kmalloc(sizeof(FILE));
	if(stream2 == NULL){
		kprintf("vt_open(): kmalloc error 2\n");
		kfree(stream1);
		return 1;
	}
	memset(stream2, 0, sizeof(struct filefunc));
	stream2->mode = FILE_MODE_READ + FILE_MODE_WRITE;

	struct filefunc *func1;
	func1 = (struct filefunc*)kmalloc(sizeof(struct filefunc));
	if(func1 == NULL){
		kprintf("vt_open(): kmalloc error 3\n");
		kfree(stream1);
		kfree(stream2);
		return 1;
	}
	memset(func1, 0, sizeof(struct filefunc));

	struct filefunc *func2;
	func2 = (struct filefunc*)kmalloc(sizeof(struct filefunc));
	if(func2 == NULL){
		kprintf("vt_open(): kmalloc error 4\n");
		kfree(stream1);
		kfree(stream2);
		kfree(func1);
		return 1;
	}
	memset(func2, 0, sizeof(struct filefunc));

	func1->fwrite = (fwrite_t)&vt_fwrite;
	func1->fread = (fread_t)&vt_fread;
	func1->ioctl = (ioctl_t)&vt_ioctl;
	func1->fclose = (fclose_t)&vt_fclose;

	func2->fwrite = (fwrite_t)&vt_fwrite;
	func2->fread = (fread_t)&vt_fread;
	func2->ioctl = (ioctl_t)&vt_ioctl;
	func2->fclose = (fclose_t)&vt_fclose;

	stream1->func = func1;
	stream2->func = func2;

	struct stream_pair streampair;
	streampair.stream1 = stream1;
	streampair.stream2 = stream2;
	streampair.to1buf = NULL;
	streampair.to2buf = NULL;
	streampair.buf1len = 0;
	streampair.buf2len = 0;
	
	list_insert(list_end(stream_pair_list), streampair);
	
	streams[0] = stream1;
	streams[1] = stream2;
	
	return 0;
}

void vt_init(void)
{
	kprintf("vt_init()\n");
	list_init(stream_pair_list);
}


