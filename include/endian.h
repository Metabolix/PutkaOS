#ifndef _ENDIAN_H
#define _ENDIAN_H 1

void word_bytes_swap(void * str, int byte_count);
void dword_bytes_swap(void * str, int byte_count);
void qword_bytes_swap(void * str, int byte_count);

void word_bytes_swap_memcpy(void *dest, const void *src, int byte_count);
void dword_bytes_swap_memcpy(void *dest, const void *src, int byte_count);
void qword_bytes_swap_memcpy(void *dest, const void *src, int byte_count);

#endif
