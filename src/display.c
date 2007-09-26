#include <display.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <thread.h>
#include <putkaos.h>
#include <devices/devmanager.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>

struct display_str {
	int num_open;
	unsigned int cx, cy;
	struct spinlock printlock; 
	struct spinlock writelock;
	unsigned int color;
} display;

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

	if (threading_on()) {
		spinl_spin(&display.printlock);
		spinl_lock(&display.writelock);
	}

	display.cx = x;
	display.cy = y;

	display_move_cursor();
	if(threading_on()){
		spinl_unlock(&display.writelock);
	}
	return 0;
}

/*unsigned char display_get_color(void)
{
	return display.color;
}*/

void display_set_color(unsigned char c)
{
	display.color = c;
}

//täyttää halutun pätkän 0x0,0x7-floodilla
//(näyttöbuffereiden tyhjäykseen, 0x7 on oletusväri)
void fill_with_blank(char *buf, unsigned int length)
{
	int j;
	for(j=0; j<length; j++){
		*(buf + j*2 + 0) = 0;
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

//FIXME: joko tämä tai vt:n update_from_current_buf() bugaa jännästi
size_t display_fwrite(const void *buf, size_t size, size_t count, FILE *stream)
{
	unsigned int i;
	char *cbuf = (char*)buf;
	for(i=0; i < count/2; i++){
		*(char*)(0xB8000 + display.cy * DISPLAY_MEM_W + display.cx * 2) = *cbuf;
		*(char*)(0xB8000 + display.cy * DISPLAY_MEM_W + display.cx * 2 + 1) = *(cbuf+1);
		display.cx++;
		if (display.cx >= DISPLAY_W) {
			display.cx = 0;
			display.cy++;
		}
		if (display.cy >= DISPLAY_H) {
			display.cy = 0;
		}
		cbuf+=2;
	}
	return count;
}

/*size_t display_fread(void *buf, size_t size, size_t count, FILE *stream)
{
}*/

int display_ioctl(FILE *stream, int request, uintptr_t param)
{
	if(request == IOCTL_DISPLAY_GET_INFO){
		struct displayinfo *di = (struct displayinfo*)param;
		if(di==NULL) return 1;
		di->w = DISPLAY_W;
		di->h = DISPLAY_H;
		return 0;
	}
	if(request == IOCTL_DISPLAY_LOCATE){
		unsigned int *xy = (unsigned int*)param;
		if(xy==NULL) return 1;
		display.cx = xy[0];
		display.cy = xy[1];
		display_move_cursor();
		return 0;
	}
	if(request == IOCTL_DISPLAY_CLS){
		display_cls();
		return 0;
	}
	if(request == IOCTL_DISPLAY_ROLL_UP){
		if(param > DISPLAY_H) param = DISPLAY_H;
		//rullataan ruutua ylös päin paramin verran rivejä
		memmove((char *)0xB8000, (char *)0xB8000 + param * DISPLAY_MEM_W,
				(DISPLAY_H - param) * DISPLAY_MEM_W);
		fill_with_blank((char *)0xB8000 + (DISPLAY_H - param) * DISPLAY_MEM_W,
				DISPLAY_W * param);
		display.cy = DISPLAY_H - 1;
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
		kprintf("display_open(): invalid mode");
		return NULL;
	}
	FILE *stream = (FILE*)kmalloc(sizeof(FILE));
	if(stream == NULL){
		kprintf("display_open(): malloc failed (1)");
		return NULL;
	}
	memset(stream, 0, sizeof(struct filefunc));
	stream->mode = FILE_MODE_WRITE;
	struct filefunc *func;
	func = (struct filefunc*)kmalloc(sizeof(struct filefunc));
	if(func == NULL){
		kfree(stream);
		kprintf("display_open(): malloc failed (2)");
		return NULL;
	}
	memset(func, 0, sizeof(struct filefunc));

	func->fwrite = (fwrite_t)&display_fwrite;
	//func->fread = (fread_t)&display_fread;
	func->ioctl = (ioctl_t)&display_ioctl;
	func->fclose = (fclose_t)&display_fclose;

	stream->func = func;
	
	if(display.num_open == 0) cls();

	display.num_open++;

	kprintf("display_open(): opened\n");

	return stream;
}

int display_remove(DEVICE *device)
{
	return 0;
}

void display_init(void) {
	memset(&display, 0, sizeof(display));
	display.color = 0;
	
	char name_[] = "display";
	int namelen = strlen(name_);
	//int r;

	//tehdään device ja tungetaan se device_insertille
	
	DEVICE *dev = (DEVICE*)kmalloc(sizeof(DEVICE));
	memset(dev, 0, sizeof(DEVICE));

	char *name = (char*)kmalloc(sizeof(char)*namelen+1);
	sprintf(name, "%s", name_);

	dev->name = name;
	dev->dev_class = DEV_CLASS_OTHER;
	dev->dev_type = DEV_TYPE_OTHER;
	dev->devopen = (devopen_t)&display_open;
	dev->remove = (devrm_t)&display_remove;
	/*r =*/ device_insert(dev);
}

