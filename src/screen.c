#include <screen.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <thread.h>
#include <putkaos.h>

struct vt_t vt[VT_COUNT];
unsigned int cur_vt = 0;
char change_to_vt = 0;

void do_eop(void)
{
	/* if user wants to change vt while printing we queue it */
	if (change_to_vt) {
		change_vt(change_to_vt);
	}
}

unsigned int vt_out_get(void)
{
	if (threading_on()) {
		return active_process_ptr->vt_num;
	}

	return VT_KERN_LOG;
}

int move(unsigned int y, unsigned int x)
{
	if (y > 25 || x > 80) {
		return -1;
	}

	unsigned int vt_num = vt_out_get();
	if (vt_num >= VT_COUNT) {
		return 0;
	}
	if (threading_on()) {
		spinl_spin(&vt[vt_num].printlock);
		spinl_lock(&vt[vt_num].writelock);
	}

	vt[vt_num].cx = x;
	vt[vt_num].cy = y;

	move_cursor();
	if(threading_on())
		spinl_unlock(&vt[vt_num].writelock);
	do_eop();
	return 0;
}

void scroll_buffer(char * p, size_t count)
{
	if (vt[cur_vt].buffer) {
		memmove(vt[cur_vt].buffer, vt[cur_vt].buffer + count, SCREEN_BUFFER_SIZE - count);
		memmove(vt[cur_vt].buffer + SCREEN_BUFFER_SIZE - count, p, count);
	}
}

unsigned char get_colour(void)
{
	unsigned int vt_num = vt_out_get();
	if (vt_num >= VT_COUNT) {
		return 0;
	}
	return vt[vt_num].colour;
}

void set_colour(unsigned char c)
{
	unsigned int vt_num = vt_out_get();
	if (vt_num >= VT_COUNT) {
		return 0;
	}
	vt[vt_num].colour = c;
}

void change_vt(unsigned int vt_num)
{
	if (vt_num >= VT_COUNT) {
		return;
	}
	if (cur_vt == vt_num) {
		return;
	}
	if (spinl_locked(&vt[cur_vt].writelock) || spinl_locked(&vt[cur_vt].printlock)) {
		change_to_vt = vt_num;
		return;
	}
	change_to_vt = 0;
	cur_vt = vt_num;
	move_cursor();
	memcpy((void*)0xB8000, vt[cur_vt].buffer + SCREEN_BUFFER_SIZE - SCREEN_SIZE, SCREEN_SIZE);
	do_eop();
}
/* FIXME */
#if 0
void scroll(int lines) {
	vt[cur_vt].scroll += lines;
	/*kprintf("%d, vt[cur_vt].scroll\n", vt[cur_vt].scroll);*/
	if((vt[cur_vt].scroll * 160) >= SCREEN_BUFFER_SIZE - SCREEN_SIZE)
		vt[cur_vt].scroll = (SCREEN_BUFFER_SIZE - SCREEN_SIZE)/160;
	if(vt[cur_vt].scroll < 0)
		vt[cur_vt].scroll = 0;
	/*kprintf("%d, vt[cur_vt].scroll\n", vt[cur_vt].scroll);*/
	memcpy((void*)0xB8000, vt[cur_vt].buffer + SCREEN_BUFFER_SIZE - SCREEN_SIZE - (vt[cur_vt].scroll * 160), SCREEN_SIZE);
}
#endif

void cls(void)
{
	int a = 0;
	unsigned int vt_num = vt_out_get();
	if (vt_num >= VT_COUNT) {
		return 0;
	}

	while (a < SCREEN_SIZE) {
		*(char*)(0xB8000 + a++) = ' ';
		*(char*)(0xB8000 + a++) = 0x7;
	}
	scroll_buffer((char*)0xB8000, SCREEN_SIZE);
	vt[vt_num].cx = 0;
	vt[vt_num].cy = 0;
}

int print(const char * string)
{
	unsigned int vt_num = vt_out_get();
	if (vt_num >= VT_COUNT) {
		return 0;
	}
	if (!vt[vt_num].in_kprintf) {
		if (threading_on()) {
			spinl_lock(&vt[vt_num].printlock);
		}
	}

	char *s = (char *)string;
	while (*s) {
		putch_vt(*s, vt_num);
		++s;
	}

	if (!vt[vt_num].in_kprintf) {
		if (threading_on()) {
			spinl_unlock(&vt[vt_num].printlock);
		}
	}

	return s - string;
}

void putch_vt(int c, unsigned int vt_num)
{
	if (vt_num >= VT_COUNT) {
		return;
	}
	if (threading_on()) {
		spinl_lock(&vt[vt_num].writelock);
	}

	if (vt[vt_num].scroll) {
		vt[vt_num].scroll = 0;
		//scroll(0);
	}
	if (c == '\b') { /* backspace */
		if (vt[vt_num].cx > 0) {
			vt[vt_num].cx--;
		}
	}
	else if (c == '\t') { /* tab */
		vt[vt_num].cx = (vt[vt_num].cx + 8) & ~7;
	}
	else if (c == '\r') { /* return */
		vt[vt_num].cx = 0;
	}
	else if (c == '\n') { /* new line */
		vt[vt_num].cx = 0;
		vt[vt_num].cy++;
	}
	else if (c >= ' ') { /* printable character */
		if(vt_num == cur_vt) {
			*(char*)(0xB8000 + vt[vt_num].cy * 160 + vt[vt_num].cx * 2) = c;
			*(char*)(0xB8000 + vt[vt_num].cy * 160 + vt[vt_num].cx * 2 + 1) = vt[vt_num].colour;
		}

		if(vt[vt_num].buffer) {
			*(vt[vt_num].buffer + SCREEN_BUFFER_SIZE - SCREEN_SIZE + vt[vt_num].cy * 160 + vt[vt_num].cx * 2) = c;
			*(vt[vt_num].buffer + SCREEN_BUFFER_SIZE - SCREEN_SIZE + vt[vt_num].cy * 160 + vt[vt_num].cx * 2 + 1) = vt[vt_num].colour;
		}
		vt[vt_num].cx++;
	}

	if (vt[vt_num].cx >= 80) {
		vt[vt_num].cx = 0;
		vt[vt_num].cy++;
	}

	if (vt[vt_num].cy >= 25) { /* scroll screen */
		int amount = vt[vt_num].cy - 24;

		memmove((void *)0xB8000, (void *)0xB8000 + amount * 160, (25 - amount) * 160);
		memset((void *)0xB8000 + (25 - amount) * 160, 0, 160);
		scroll_buffer((char*)0xB8000 + (25 - amount) * 160, (25 - amount) * 160);
		vt[vt_num].cy = 24;
	}
	if (threading_on()) {
		spinl_unlock(&vt[vt_num].writelock);
	}
	if (vt_num == cur_vt) {
		move_cursor();
	}
	do_eop();
}

void move_cursor(void)
{
	unsigned int temp = vt[cur_vt].cy * 80 + vt[cur_vt].cx;

	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

void vts_init(void)
{
	int i;
	for(i = 0; i < VT_COUNT; i++) {
		vt[i].buffer = kmalloc(SCREEN_BUFFER_SIZE);
	}

	memcpy(vt[0].buffer + SCREEN_BUFFER_SIZE - SCREEN_SIZE, (const void *)0xB8000, SCREEN_SIZE);
}

void screen_init(void) {
	int i;
	for(i = 0; i < VT_COUNT; i++) {
		vt[i].scroll = 0;
		vt[i].in_kprintf = 0;
		vt[i].cx = vt[i].cy = 0;
		vt[i].colour = 0x7;
		vt[i].buffer = 0;
		vt[i].kb_buff_filled = 0;
		spinl_init(&vt[i].writelock);
		spinl_init(&vt[i].printlock);
	}
}
