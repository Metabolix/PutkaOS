#include <screen.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <thread.h>
#include <putkaos.h>

#include <stdio.h>
#include <string.h>

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
	if (y >= SCREEN_H || x >= SCREEN_W) {
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
	#ifdef SCREEN_BUFFER
	if (vt[cur_vt].buffer) {
		memmove(vt[cur_vt].buffer, vt[cur_vt].buffer + count, SCREEN_BUFFER_SIZE - count);
		memmove(vt[cur_vt].buffer + SCREEN_BUFFER_SIZE - count, p, count);
	}
	#endif
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
		return;
	}
	vt[vt_num].colour = c;
}

void update_from_buffer()
{
	#ifdef SCREEN_BUFFER
	memcpy((void*)0xB8000, vt[cur_vt].buffer + SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE - vt[cur_vt].scroll * SCREEN_MEM_W, SCREEN_MEM_SIZE);

	/*char temp[100];
	sprintf(temp, "(%d,%d,%d,%d)",
			*(vt[cur_vt].buffer + (SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE) + 0),
			*(vt[cur_vt].buffer + (SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE) + 1),
			*(vt[cur_vt].buffer + (SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE) + 2),
			*(vt[cur_vt].buffer + (SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE) + 3)
			);
	int i;
	for(i=0; i<strlen(temp); i++){
		*((char*)0xB8000 + i*2) = temp[i];
		*((char*)0xB8000 + i*2+1) = 0x7;
	}*/

	move_cursor();

	#endif
}

//täyttää halutun pätkän 0x0,0x7-floodilla (näyttöbuffereille kiva)
void fill_with_blank(char *buf, unsigned int length)
{
	int j;
	for(j=0; j<length; j++){
		*(buf + j*2 + 0) = 0;
		*(buf + j*2 + 1) = 0x7;
	}
}

void change_vt(unsigned int vt_num)
{
	#ifdef SCREEN_BUFFER
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
	//move_cursor();
	update_from_buffer();
	do_eop();
	#endif
}

void scroll(int lines) {
	#ifdef SCREEN_BUFFER
	vt[cur_vt].scroll += lines;
	if(vt[cur_vt].scroll < 0)
		vt[cur_vt].scroll = 0;
	if(vt[cur_vt].scroll > SCREEN_BUF_H - SCREEN_H)
		vt[cur_vt].scroll = SCREEN_BUF_H - SCREEN_H;
	update_from_buffer();
	#endif
}

void cls(void)
{
	int a = 0;
	unsigned int vt_num = vt_out_get();
	if (vt_num >= VT_COUNT) {
		return;
	}

	while (a < SCREEN_MEM_SIZE) {
		*(char*)(0xB8000 + a++) = ' ';
		*(char*)(0xB8000 + a++) = 0x7;
	}
	scroll_buffer((char*)0xB8000, SCREEN_MEM_SIZE);
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

	if (vt[vt_num].scroll != 0) {
		vt[vt_num].scroll = 0;
		scroll(0);
	}
	
	if (c == '\b') { /* backspace */
		if (vt[vt_num].cx > 0) {
			vt[vt_num].cx--;
		} else {
			vt[vt_num].cx = SCREEN_W - 1;
			vt[vt_num].cy--;
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
			*(char*)(0xB8000 + vt[vt_num].cy * SCREEN_MEM_W + vt[vt_num].cx * 2) = c;
			*(char*)(0xB8000 + vt[vt_num].cy * SCREEN_MEM_W + vt[vt_num].cx * 2 + 1) = vt[vt_num].colour;
		}
		#ifdef SCREEN_BUFFER
		if(vt[vt_num].buffer) {
			*(vt[vt_num].buffer + SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE + vt[vt_num].cy * SCREEN_MEM_W + vt[vt_num].cx * 2) = c;
			*(vt[vt_num].buffer + SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE + vt[vt_num].cy * SCREEN_MEM_W + vt[vt_num].cx * 2 + 1) = vt[vt_num].colour;
		}
		#endif
		vt[vt_num].cx++;
	}

	if (vt[vt_num].cx >= SCREEN_W) {
		vt[vt_num].cx = 0;
		vt[vt_num].cy++;
	}

	if (vt[vt_num].cy >= SCREEN_H) { /* scroll screen */
		int amount = vt[vt_num].cy - (SCREEN_H - 1);

		memmove((char *)0xB8000, (char *)0xB8000 + amount * SCREEN_MEM_W,
				(SCREEN_H - amount) * SCREEN_MEM_W);
		
		fill_with_blank((char *)0xB8000 + (SCREEN_H - amount) * SCREEN_MEM_W, SCREEN_W);

		scroll_buffer((char*)0xB8000 + (SCREEN_H - amount) * SCREEN_MEM_W,
				amount * SCREEN_MEM_W);

		vt[vt_num].cy = SCREEN_H - 1;
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
	unsigned int temp = (vt[cur_vt].cy + vt[cur_vt].scroll) * SCREEN_W + vt[cur_vt].cx;

	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

void vts_init(void)
{
	int i, j;
	for(i = 0; i < VT_COUNT; i++) {
		vt[i].buffer = (char*)kmalloc(SCREEN_BUFFER_SIZE);
		for(j=0; j<SCREEN_BUF_H; j++){
			fill_with_blank(vt[i].buffer, SCREEN_W);
		}
		//memset(vt[i].buffer, 0, SCREEN_BUFFER_SIZE);
	}

	memcpy(vt[0].buffer + SCREEN_BUFFER_SIZE - SCREEN_MEM_SIZE, (const void *)0xB8000, SCREEN_MEM_SIZE);
}

void screen_init(void) {
	int i;
	for (i = 0; i < VT_COUNT; i++) {
		vt[i].scroll = 0;
		vt[i].in_kprintf = 0;
		vt[i].cx = vt[i].cy = 0;
		vt[i].colour = 0x7;
		vt[i].buffer = 0;
		vt[i].kb_buf_count = 0;
		vt[i].kb_buf_start = 0;
		vt[i].kb_buf_end = 0;
		spinl_init(&vt[i].writelock);
		spinl_init(&vt[i].printlock);
	}
}
