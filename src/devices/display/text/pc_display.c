/*
 * tekstinäyttörajapinnan mukainen pc:n tekstitilaa käyttävä ajuri
 * celeron55
 */

#include <devices/display/text/pc_display.h>
#include <io.h>
#include <irq.h>
#include <multitasking/multitasking.h>
#include <devices/devmanager.h>
#include <stdio.h>
#include <string.h>
#include <kprintf.h>
#include <vt.h>

struct display_str {
	int num_open;
	unsigned int cx, cy;
	unsigned int color;
} display;

void display_move_cursor(void);
int display_locate(unsigned int y, unsigned int x);
void fill_with_blank(char *buf, unsigned int length);
void display_cls(void);
size_t display_fwrite(const void *buf, size_t size, size_t count, FILE *stream);
int display_ioctl(FILE *stream, int request, uintptr_t param);
int display_fclose(FILE *stream);
FILE *display_open(DEVICE *device, uint_t mode);
int display_remove(DEVICE *device);
void display_init(void);

DEVICE pc_display_dev = {
	.name = "display",
	.dev_class = DEV_CLASS_DISPLAY,
	.dev_type = DEV_TYPE_TEXT_DISPLAY,
	// .index
	.devopen = (devopen_t) display_open,
	.remove = (devrm_t) display_remove,
};

void display_move_cursor(void)
{
	unsigned int temp = display.cy * DISPLAY_W + display.cx;

	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

int display_locate(unsigned int y, unsigned int x)
{
	if (y >= DISPLAY_H || x >= DISPLAY_W) {
		return -1;
	}

	display.cx = x;
	display.cy = y;

	return 0;
}

//täyttää halutun pätkän 0x0,0x7-floodilla
//(näyttöbuffereiden tyhjäykseen, 0x7 on oletusväri)
void fill_with_blank(char *buf, unsigned int length)
{
	int j;
	for(j=0; j<length; j++){
		*(buf + j*2 + 0) = ' ';
		*(buf + j*2 + 1) = 0x7;
	}
}

void display_cls(void)
{
	int a = 0;
	fill_with_blank((char*)(0xB8000 + a++), DISPLAY_MEM_SIZE/2);
	display.cx = 0;
	display.cy = 0;
}

size_t display_fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	//- Tarkoitus on tunkea merkkejä ruudulle peräkkäin, aloittaen kursorin
	//  nykyisestä paikasta ja wräppäytyen aina reunan jälkeen uudelle riville.
	//- Merkkejä ei tule enempää, kuin mitä kursorista näyttöbufferin loppuun
	//  mahtuu.
	//- Kursori pitää siirtää piirrettyjen merkkien perään.
	//- buffissa tulee merkki-väri-pareja peräkkäin.
	//- count on bufferin koko tavuina ja size pitäisi olla sizeof(char) eli 1.
	//- Bufferin koko on aina parillinen
	//- Näyttöä ohjaava juttu (vt) pitää huolen ettei mennä bufferin yli.

	//Tosin voi sen tietenkin tarkistaa...
	if(display.cy * DISPLAY_MEM_W + display.cx*2 + count > DISPLAY_MEM_SIZE)
		count = DISPLAY_MEM_SIZE - (display.cy * DISPLAY_MEM_W + display.cx*2);

	//koska ei mennä bufferin yli, voidaan kopioida buf vaan suoraan paikalleen.
	memcpy((char*)(0xB8000 + display.cy * DISPLAY_MEM_W + display.cx * 2), buf,
			size*count);

	//ja kursori pitää asettaa uuteen paikkaansa.
	display.cx += size*count/2;
	int ychange = display.cx / DISPLAY_W;
	display.cx -= ychange * DISPLAY_W;
	display.cy += ychange;
	display.cy %= DISPLAY_H;
	display_move_cursor();

	//palautetaan piirrettyjen tavujen määrä
	return count;
}

/*size_t display_fread(void *buf, size_t size, size_t count, FILE *stream)
{
}*/

int display_ioctl(FILE *stream, int request, uintptr_t param)
{
	if(request == IOCTL_DISPLAY_GET_INFO){
		//täytetään paramin osoittama displayinfo-strukti.
		struct displayinfo *di = (struct displayinfo*)param;
		if(di==NULL) return 1;
		di->w = DISPLAY_W;
		di->h = DISPLAY_H;
		return 0;
	}
	if(request == IOCTL_DISPLAY_LOCATE){
		//- Siirretään kursori paramin osoittaman int-taulukon
		//  mukaiseen paikkaan
		//- Jos paikka ei ole näytön alueella, kursori piilotetaan.
		int *xy = (int*)param;
		if(xy==NULL) return 1;
		display.cx = xy[0];
		display.cy = xy[1];
		display_move_cursor();
		return 0;
	}
	if(request == IOCTL_DISPLAY_CLS){
		//- Tyhjätään koko ruutu (' '-merkkejä) (värillä ei väliä)
		//- Kursorin paikka ruudun vasempaan yläkulmaan (0,0)
		display_cls();
		return 0;
	}
	if(request == IOCTL_DISPLAY_ROLL_UP){
		//- Rullataan ruutua ylöspäin paramin verran rivejä.
		//- Uudet rivit täytetään tyhjällä (' '-merkeillä) (värillä ei väliä)
		//- Rivien määrä on (unsigned int)param.
		//- param <= ilmoitettu näytön korkeus
		//- Kursorin paikka kerrotaan aina tämän ioctl:n jälkeen toisella.
		memmove((char *)0xB8000, (char *)0xB8000 + param * DISPLAY_MEM_W,
				(DISPLAY_H - param) * DISPLAY_MEM_W);
		fill_with_blank((char *)0xB8000 + (DISPLAY_H - param) * DISPLAY_MEM_W,
				DISPLAY_W * param);
		return 0;
	}
	return 1;
}

int display_fclose(FILE *stream)
{
	if(display.num_open==0) return 1;
	if(stream){
		if(stream->func) kfree((void*)stream->func);
		kfree(stream);
	}
	display.num_open--;
	kprintf("display_fclose(): closed\n");
	return 0;
}

FILE *display_open(DEVICE *device, uint_t mode)
{
	if(!(mode & FILE_MODE_WRITE) || (mode & FILE_MODE_READ)){
		kprintf("display_open(): invalid mode\n");
		return NULL;
	}
	FILE *stream = (FILE*)kmalloc(sizeof(FILE));
	if(stream == NULL){
		kprintf("display_open(): malloc failed (1)\n");
		return NULL;
	}
	memset(stream, 0, sizeof(struct filefunc));
	stream->mode = FILE_MODE_WRITE;
	struct filefunc *func;
	func = (struct filefunc*)kmalloc(sizeof(struct filefunc));
	if(func == NULL){
		kfree(stream);
		kprintf("display_open(): malloc failed (2)\n");
		return NULL;
	}
	memset(func, 0, sizeof(struct filefunc));

	func->fwrite = (fwrite_t)&display_fwrite;
	//func->fread = (fread_t)&display_fread;
	func->ioctl = (ioctl_t)&display_ioctl;
	func->fclose = (fclose_t)&display_fclose;

	stream->func = func;

	display.num_open++;

	kprintf("display_open(): opened\n");

	return stream;
}

int display_remove(DEVICE *device)
{
	return 0;
}

void display_init(void)
{
	memset(&display, 0, sizeof(display));

	switch (device_insert(&pc_display_dev)) {
		case 0:
			break;
		case DEV_ERR_TOTAL_FAILURE:
			kprintf("display_init(): unknown error inserting device\n");
			break;
		case DEV_ERR_BAD_NAME:
			kprintf("display_init(): error inserting device: bad name\n");
			break;
		case DEV_ERR_EXISTS:
			kprintf("display_init(): error inserting device: device exists\n");
			break;
		case DEV_ERR_BAD_STRUCT:
			kprintf("display_init(): error inserting device: bad info struct\n");
			break;
	}
}
