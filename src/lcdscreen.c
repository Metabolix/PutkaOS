#include <lcdscreen.h>
#include <io.h>
#include <timer.h>

/* 0123 data ylemm� bitit
   4    _Instruction/Register Select
   5    E Clock */

int lcd_port;

void inst4(char plorp)
{
	//ensin enable clock ylhäällä ja sitten alhaalla
	//päästetään läpi bitit 0001 1111 (RS ja data)
	outportb(lcd_port, (0x1f&plorp)|0x20);
	kwait(0, 1000 * 1);
	outportb(lcd_port, (0x1f&plorp));
	//kwait(1);
}

void inst8(char ploo)
{
	//ylemmät bitit ja alemmat bitit erikseen
	inst4(0x0f & (ploo>>4));
	inst4(0x0f & (ploo   ));
}

void reg8(char ploo)
{
	//ylemmät bitit ja alemmat bitit erikseen, registeribitti ylhäällä
	inst4((0x0f & (ploo>>4)) | 0x10);
	inst4((0x0f & (ploo   )) | 0x10);
}

void init(void)
{
	//jotain hassuja inittejä
	inst4(0x03);
	kwait(0, 1000 * 5);
	inst4(0x03);
	kwait(0, 1000 * 1);
	inst4(0x03);
	kwait(0, 1000 * 1);
	//interface length, vain ylemmät bitit => 0010 0000
	// 001[0 00]00 = 4 bittinen, 1 rivi, 5x7-merkit
	inst4(0x02);
	//tästä lähtien joka käsky kahdessa osassa (inst8 palastelee käskyn)
}

void Clear(void)
{
	inst8(0x01);
}

void ReturnToHome(void)
{
	inst8(0x02);
}

void MoveMode(int bautomove, int bshiftdisplay)
{
	inst8( 0x04 | (bautomove<<3) | (bshiftdisplay<<2) );
}

void Enable(int bdisplay, int bcursor, int bblink)
{
	inst8( 0x08 | (bdisplay<<2) | (bcursor<<1) | (bblink) );
}

void ShiftMode(int bshiftdisplay, int bdir)
{
	inst8( 0x10 | (bshiftdisplay<<3) | (bdir<<2) );
}

void IfaceLen(int b8bits, int b2rows, int b5x10)
{
	inst8( 0x20 | (b8bits<<4) | (b2rows<<3) | (b5x10<<2) );
}

void MoveToCGRAM(char paikka)
{
	// 0x40 = 0100 0000
	// 0x3f = 0011 1111
	inst8( 0x40 | (0x3f & paikka) );
}

void MoveToDisplay(char paikka)
{
	// 0x80 = 1000 0000
	// 0x7f = 0111 1111
	inst8( 0x80 | (0x7f & paikka) );
}

void init2(void)
{
	IfaceLen(0, 1, 0);
	ShiftMode(0, 0);
	MoveMode(1, 0);
	Enable(1, 1, 1);
}

void lcd_cls(void)
{
	Clear();
}

void lcd_move_cursor(void)
{
}

void lcd_putch(int c){
	reg8(c);
}

int lcd_move(unsigned int y, unsigned int x){
	char p;
	if(y >= 2 || x >= 16){
		return -1;
	}
	if(y == 0) p = 0;
	else p = 0x40;
	p += x;
	MoveToDisplay(p);
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

