#include <lcdscreen.h>
#include <io.h>
#include <timer.h>

/* 0123 data ylemmät bitit
   4    _Instruction/Register Select
   5    E Clock */

int lcd_port;

void inst4(char plorp)
{
	outportb(lcd_port, (0x0f&plorp)|32);
	kwait(1);
	outportb(lcd_port, (0x0f&plorp));
	kwait(1);
}
 
void inst8(char ploo)
{
	inst4(ploo>>4);
	inst4(ploo);
}

void glxdata(char lol) {
	outportb(lcd_port, (0x0f&(lol>>4))|48);
	kwait(5);
	outportb(lcd_port, (0x0f&(lol>>4))|16);
	kwait(5);
	outportb(lcd_port, (0x0f&lol)|48);
	kwait(5);
	outportb(lcd_port, (0x0f&lol)|16);
	kwait(5);
}

void init(void)
{
	inst4(0x03);
	kwait(5);
	inst4(0x03);
	kwait(1);
	inst4(0x03);
	kwait(1);
	//laitetaan 4-bittinen moodi
	inst4(0x02);
	//tästä lähtien joka käsky kahdessa osassa (inst8 palastelee käskyn)
}

void init2(void)
{
	//set interface length
	inst8(0x28);
	//turn off the Display
	inst8(0x10);
	//Set Cursor Move Direction
	inst8(0x06);
	//Enable Display/Cursor
	inst8(0x0C);
}


void place(char paikka)
{
	// 0x80 = 1000 0000
	// 0x7f = 0111 1111
	inst8( 0x80 | (0x7f & paikka) );
}

void lcd_cls(void)
{
	inst8(0x01);
	
}

void lcd_move_cursor(void)
{
}

void lcd_putch(int c){
	glxdata(c);
}

int lcd_move(unsigned int y, unsigned int x){
	char p;
	if(y >= 2 || x >= 16){
		return -1;
	}
	if(y == 0) p = 0;
	else p = 0x40;
	p += x;
	place(p);
	return 0;
}

void lcd_set_color(unsigned char c){
}

unsigned char lcd_get_color(){
	return 0;
}

void lcd_init(int port)
{
	lcd_port = port;
	init();
	init2();
	lcd_cls();
}

