#include <lcdscreen.h>
#include <io.h>
#include <timer.h>
//#include <screen.h>

/* 0123 data ylemm� bitit
   4    _Instruction/Register Select
   5    E Clock */

int started = 0, lcd_count, lcd_mode, lcd_port;
char buffer[LCD_ROWS*3][LCD_COLS*3];
int cy, cx, cid;
int lcdh, lcdw;
int moving_needed;
int lcdlock = 0;

void inst4(char plorp, int id)
{
	char orraus;
	if(!started) return;
	if(id==0) orraus = 0x20;
	else if(id==1) orraus = 0x40;
	else if(id==2) orraus = 0x80;
	else if(id==-1) orraus = 0x20+0x40+0x80; //kaikki
	else{
		//kprintf("lcdscreen: inst4: invalid screen id %i\n", id);
		return;
	}
	//ensin enable clock ylhäällä ja sitten alhaalla
	//päästetään läpi bitit 0001 1111 (RS ja data)
	outportb(lcd_port, (0x1f&plorp)|orraus);
	kwait(0, 1);
	outportb(lcd_port, (0x1f&plorp));
	kwait(0, 1);
}
 
void inst8(char ploo, int id)
{
	//ylemmät bitit ja alemmat bitit erikseen
	inst4(0x0f & (ploo>>4), id);
	inst4(0x0f & (ploo   ), id);
}

void reg8(char ploo, int id)
{
	//ylemmät bitit ja alemmat bitit erikseen, registeribitti ylhäällä
	inst4((0x0f & (ploo>>4)) | 0x10, id);
	inst4((0x0f & (ploo   )) | 0x10, id);
}

void init(int id)
{
	//jotain hassuja inittejä
	inst4(0x03, id);
	kwait(0, 5000);
	inst4(0x03, id);
	kwait(0, 1000);
	inst4(0x03, id);
	kwait(0, 1000);
	//interface length, vain ylemmät bitit => 0010 0000
	// 001[0 00]00 = 4 bittinen, 1 rivi, 5x7-merkit
	inst4(0x02, id);
	//tästä lähtien joka käsky kahdessa osassa (inst8 palastelee käskyn)
}

void Clear(int id)
{
	inst8(0x01, id);
}

void ReturnToHome(int id)
{
	inst8(0x02, id);
}

void MoveMode(int bautomove, int bshiftdisplay, int id)
{
	inst8( 0x04 | (bautomove<<3) | (bshiftdisplay<<2), id);
}

void Enable(int bdisplay, int bcursor, int bblink, int id)
{
	inst8( 0x08 | (bdisplay<<2) | (bcursor<<1) | (bblink), id);
}

void Shift(int bshiftdisplay, int bdirright, int id)
{
	inst8( 0x10 | (bshiftdisplay<<3) | (bdirright<<2), id);
}

void IfaceLen(int b8bits, int b2rows, int bfont5x10, int id)
{
	inst8( 0x20 | (b8bits<<4) | (b2rows<<3) | (bfont5x10<<2), id);
}

void MoveToCGRAM(char paikka, int id)
{
	// 0x40 = 0100 0000
	// 0x3f = 0011 1111
	inst8( 0x40 | (0x3f & paikka), id);
}

void MoveToDisplay(char paikka, int id)
{
	// 0x80 = 1000 0000
	// 0x7f = 0111 1111
	inst8( 0x80 | (0x7f & paikka), id);
}

void init2(int id)
{
	IfaceLen(0, 1, 0, id);
	MoveMode(1, 0, id);
	Enable(1, 1, 1, id);
}

/////////////////////////////
void unlocklcd()
{
	lcdlock = 0;
}

void locklcd()
{
	while(lcdlock);
	lcdlock = 1;
}

/////////////////////////////

int Move(unsigned int y, unsigned int x, int id)
{
	char p;
	if(y >= lcdh || x >= lcdw){
		return -1;
	}
	if(y == 0) p = 0x00;
	else p = 0x40;
	//else if(y==2) p = 0x20; //ei tueta kuin kaksirivisiä
	//else if(y==3) p = 0x60;
	p += x;
	MoveToDisplay(p, id);
	return 0;
}

/////////////////////////////

void HideCursor()
{
	Enable(1, 0, 0, -1);
}

void UpdateCursor()
{
	Enable(1, 0, 0, -1);
	Enable(1, 1, 1, cid);
}

int FullMove(unsigned int y, unsigned int x)
{
	if(lcd_mode==0){
		cid = x / LCD_COLS;
		x -= cid * LCD_COLS;
		cy = y;
		cx = cid * LCD_COLS + x;
	}
	else if(lcd_mode==1){
		cid = y / LCD_ROWS;
		y -= cid * LCD_ROWS;
		cx = x;
		cy = cid * LCD_ROWS + y;
	}
	if(Move(y, x, cid) == -1) return -1;
	UpdateCursor();
	moving_needed = 0;
	return 0;
}

void FullPutchar(char c){
	if(lcd_mode==0){
		if(cx >= (cid + 1) * LCD_COLS || moving_needed){
			FullMove(cy, cx);
		}
	}
	reg8(c, cid);
	buffer[cy][cx] = c;
	cx++;
	if(lcd_mode==0){
		//jos oltiin rivin lopussa, ja vielä on jäljellä näyttöjä, kursori näkyviin
		if(cx == (cid+1) * LCD_COLS) FullMove(cy, cx);
	}
}

void FullClear(){
	Clear(-1);
	moving_needed = 1;
}

void FullClearBuffer(){
	int y, x;
	for(y = 0; y < lcdh; y++){
		for(x = 0; x < lcdw; x++){
			buffer[y][x] = ' ';
		}
	}
	moving_needed = 1;
}

void FullClearAll(){
	FullClear();
	FullClearBuffer();
	FullMove(0,0);
}

void FullDrawBufferLine(int y)
{
	int x;
	FullMove(y, 0);
	for(x = 0; x < lcdw; x++){
		FullPutchar(buffer[y][x]);
	}
	moving_needed = 1;
}

void FullDrawBuffer()
{
	int y;
	for(y = lcdh-1; y >= 0; y--){ // väärin päin, niin tulee nätimpi
		FullDrawBufferLine(y);
	}
	moving_needed = 1;
}

void FullScrollBufferUpAndDraw(){
	int y, x;
	HideCursor();
	//bufferin rivit ylöspäin
	for(y = 0; y < lcdh - 1; y++){
		for(x = 0; x < lcdw; x++){
			buffer[y][x] = buffer[y+1][x];
		}
	}
	//viimeinen rivi tyhjäksi
	for(x=0; x < lcdw; x++){
		buffer[lcdh-1][x] = ' ';
	}
	//tyhjätään näyttö ja piirretään kaikki paitsi viimeinen rivi
	//(nopeampaa kuin koko bufferin uudelleenpiirtäminen)
	FullClear();
	for(y=0; y < lcdh-1; y++){
		FullDrawBufferLine(y);
	}
	UpdateCursor();
}

/////////////////////////////

void lcd_cls(void)
{
	locklcd();
	FullClearAll();
	unlocklcd();
}

void lcd_move_cursor(void)
{
}

int lcd_move(unsigned int y, unsigned int x){
	int r;
	locklcd();
	r = FullMove(y, x);
	unlocklcd();
	return r;
}

void lcd_putch(int c){
	locklcd();
	if(c == '\b'){
		cx--;
		moving_needed = 1;
	}
	else if(c == '\n'){
		if(cx > 0){ // ettei tuhlata kallisarvoisia rivejä
			cy++;
			cx = 0;
			moving_needed = 1;
		}
	}
	else if(c == '\t'){
		cx = (cx + 8) & ~7;
		moving_needed = 1;
	}
	else if(c == '\r'){
		cx = 0;
		moving_needed = 1;
	}
	else if(c >= ' '){
		FullPutchar(c);
	}
	
	if(cx >= lcdw){
		cy++;
		cx = 0;
		moving_needed = 1;
	}
	
	if(cx < 0){
		if(cy>0){
			cy--;
			cx = lcdw-1;
		}
		else cx = 0;
		moving_needed = 1;
	}
	
	if(cy >= lcdh){
		FullScrollBufferUpAndDraw();
		FullMove(lcdh - 1, 0);
	}
	
	if(moving_needed) FullMove(cy, cx);
	
	unlocklcd();
}

void lcd_set_color(unsigned char c){
}

unsigned char lcd_get_color(){
	return 0;
}

void lcd_init(int port, int count, int mode)
{
	if(count==0) return;
	
	locklcd();
	
	started = 1;
	
	lcd_port = port;
	lcd_count = count;
	lcd_mode = mode;
	if(lcd_mode==0){
		lcdh = LCD_ROWS;
		lcdw = count * LCD_COLS;
	}
	else if(lcd_mode==1){
		lcdh = count * LCD_ROWS;
		lcdw = LCD_COLS;
	}
	
	init(-1);
	init2(-1);
	
	FullClearAll();
	
	unlocklcd();
}

