#include <screen.h>
#include <vt.h>

void cls(void)
{
	vt_cls(vt_out_get());
}
int putch(char c)
{
	return vt_putch(vt_out_get(), c);
}
int print(const char * string)
{
	return vt_print(vt_out_get(), string);
}
int move(unsigned int y, unsigned int x)
{
	return  vt_locate(vt_out_get(), y, x);
}
unsigned char get_colour(void)
{
	return vt_get_color(vt_out_get());
}
void set_colour(unsigned char c)
{
	vt_set_color(vt_out_get(), c);
}

