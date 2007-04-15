#include <endian.h>

void word_bytes_swap(void *str, int byte_count)
{
	char temp;
	for (byte_count ^= (byte_count & 1); byte_count; byte_count -= 2) {
		temp = ((char*)str)[byte_count - 1];
		((char*)str)[byte_count - 1] = ((char*)str)[byte_count - 2];
		((char*)str)[byte_count - 2] = temp;
	}
}

void dword_bytes_swap(void *str, int byte_count)
{
	char temp;
	for (byte_count ^= (byte_count & 3); byte_count; byte_count -= 4) {
		temp = ((char*)str)[byte_count - 2];
		((char*)str)[byte_count - 2] = ((char*)str)[byte_count - 3];
		((char*)str)[byte_count - 3] = temp;
		temp = ((char*)str)[byte_count - 1];
		((char*)str)[byte_count - 1] = ((char*)str)[byte_count - 4];
		((char*)str)[byte_count - 4] = temp;
	}
}

void qword_bytes_swap(void *str, int byte_count)
{
	char temp;
	for (byte_count ^= (byte_count & 7); byte_count; byte_count -= 8) {
		temp = ((char*)str)[byte_count - 4];
		((char*)str)[byte_count - 4] = ((char*)str)[byte_count - 5];
		((char*)str)[byte_count - 5] = temp;
		temp = ((char*)str)[byte_count - 3];
		((char*)str)[byte_count - 3] = ((char*)str)[byte_count - 6];
		((char*)str)[byte_count - 6] = temp;
		temp = ((char*)str)[byte_count - 2];
		((char*)str)[byte_count - 2] = ((char*)str)[byte_count - 7];
		((char*)str)[byte_count - 7] = temp;
		temp = ((char*)str)[byte_count - 1];
		((char*)str)[byte_count - 1] = ((char*)str)[byte_count - 8];
		((char*)str)[byte_count - 8] = temp;
	}
}

void word_bytes_swap_memcpy(void *dest, const void *src, int byte_count)
{
	for (byte_count ^= (byte_count & 1); byte_count; byte_count -= 2) {
		((char*)dest)[byte_count - 1] = ((const char*)src)[byte_count - 2];
		((char*)dest)[byte_count - 2] = ((const char*)src)[byte_count - 1];
	}
}
void dword_bytes_swap_memcpy(void *dest, const void *src, int byte_count)
{
	for (byte_count ^= (byte_count & 1); byte_count; byte_count -= 4) {
		((char*)dest)[byte_count - 1] = ((const char*)src)[byte_count - 4];
		((char*)dest)[byte_count - 2] = ((const char*)src)[byte_count - 3];
		((char*)dest)[byte_count - 3] = ((const char*)src)[byte_count - 2];
		((char*)dest)[byte_count - 4] = ((const char*)src)[byte_count - 1];
	}
}
void qword_bytes_swap_memcpy(void *dest, const void *src, int byte_count)
{
	for (byte_count ^= (byte_count & 1); byte_count; byte_count -= 8) {
		((char*)dest)[byte_count - 1] = ((const char*)src)[byte_count - 8];
		((char*)dest)[byte_count - 2] = ((const char*)src)[byte_count - 7];
		((char*)dest)[byte_count - 3] = ((const char*)src)[byte_count - 6];
		((char*)dest)[byte_count - 4] = ((const char*)src)[byte_count - 5];
		((char*)dest)[byte_count - 5] = ((const char*)src)[byte_count - 4];
		((char*)dest)[byte_count - 6] = ((const char*)src)[byte_count - 3];
		((char*)dest)[byte_count - 7] = ((const char*)src)[byte_count - 2];
		((char*)dest)[byte_count - 8] = ((const char*)src)[byte_count - 1];
	}
}
