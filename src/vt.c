#include <vt.h>
#include <screen.h>

int initialized = 0;

struct vt_t vt[VT_COUNT];
unsigned int cur_vt;
int change_to_vt;

FILE *driverstream = NULL;
struct displayinfo driverinfo;

void vt_fallback_update_cursor()
{
	if(!initialized) return;
	unsigned int temp = vt[cur_vt].cy * 80 + vt[cur_vt].cx;
	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

void vt_locate_display(unsigned int x, unsigned int y)
{
	if(!initialized) return;
	if(!driverstream) return;
	unsigned int xy[2];
	xy[0] = x;
	xy[1] = y;
	ioctl(driverstream, IOCTL_DISPLAY_LOCATE, (uintptr_t)&xy);
}

void vt_update_display_cursor()
{
	vt_locate_display(vt[cur_vt].cx, vt[cur_vt].cy + vt[cur_vt].scroll);
}

//täyttää halutun pätkän 0x0,0x7-floodilla
//(näyttöbuffereiden tyhjäykseen, 0x7 on oletusväri)
void vt_fill_with_blank(char *buf, unsigned int length)
{
	int j;
	for(j=0; j<length; j++){
		*(buf + j*2 + 0) = 0;
		*(buf + j*2 + 1) = 0x7;
	}
}

unsigned char vt_get_color(unsigned int vt_num)
{
	if(!initialized) return -1;
	if (vt_num >= VT_COUNT) {
		return 0;
	}
	return vt[vt_num].color;
}

void vt_set_color(unsigned int vt_num, unsigned char c)
{
	if(!initialized) return;
	if (vt_num >= VT_COUNT) {
		return;
	}
	vt[vt_num].color = c;
}

int vt_locate(unsigned int vt_num, unsigned int x, unsigned int y)
{
	if(!initialized) return 1;
	if(vt_num >= VT_COUNT) return 1;
	int ret = 0;
	if(x >= driverinfo.w){
		x = driverinfo.w - 1;
		ret = 1;
	}
	if(y >= driverinfo.h){
		y = driverinfo.h - 1;
		ret = 1;
	}
	vt[vt_num].cx = x;
	vt[vt_num].cy = y;
	if(vt_num == cur_vt){
		if(driverstream){
			vt_update_display_cursor();
		}
		else{
			vt_fallback_update_cursor();
		}
	}

	return ret;
}

void vt_cls(unsigned int vt_num)
{
	if(!initialized) return;
	if (vt_num >= VT_COUNT) {
		return;
	}
	vt[vt_num].cx = 0;
	vt[vt_num].cy = 0;
	if(vt_num == cur_vt){
		if(driverstream){
			ioctl(driverstream, IOCTL_DISPLAY_CLS, NULL);
		}
		else{
			int i;
			for(i=0; i<80*25; i++){
				*(char*)(0xB8000 + i * 2 + 0) = ' ';
				*(char*)(0xB8000 + i * 2 + 1) = 0x7;
			}
			vt_fallback_update_cursor();
		}
	}
	if(vt[vt_num].buffer && initialized){
		vt_fill_with_blank(vt[vt_num].buffer + vt[vt_num].bufsize - driverinfo.h*vt[vt_num].bufw*2, driverinfo.h*vt[vt_num].bufw);
	}
}

int vt_print(unsigned int vt_num, const char *string)
{
	if(!initialized) return 1;
	if (vt_num >= VT_COUNT) return 1;
	if (!vt[vt_num].in_kprintf) {
		if (threading_on()) {
			spinl_lock(&vt[vt_num].printlock);
		}
	}

	char *s = (char *)string;
	while (*s) {
		vt_putch(vt_num, *s);
		++s;
	}

	if (!vt[vt_num].in_kprintf) {
		if (threading_on()) {
			spinl_unlock(&vt[vt_num].printlock);
		}
	}

	return s - string;
}

///

unsigned int vt_get_display_height(void)
{
	if(!initialized) return 0;
	if(driverstream) return driverinfo.h;
	else return 25;
}


void do_eop(void)
{
	if(!initialized) return;
	/* if user wants to change vt while printing we queue it */
	if (change_to_vt) {
		vt_change(change_to_vt);
	}
}

unsigned int vt_out_get(void)
{
	if(!initialized) return 0;
	if (threading_on()) {
		return active_process_ptr->vt_num;
	}

	return VT_KERN_LOG;
}

void update_from_current_buf(void)
{
	if(!initialized) return;
	if(driverstream){
		if(vt[cur_vt].buffer){
			vt_locate_display(0, 0);
			fwrite((char*)vt[cur_vt].buffer + vt[cur_vt].bufsize
					- (driverinfo.h * driverinfo.w * 2)
					- (vt[cur_vt].scroll * driverinfo.w * 2), 1,
					(driverinfo.h * driverinfo.w * 2), driverstream);
			vt_update_display_cursor();
			
			/*
			int x, y;
			for(y=0; y<driverinfo.h; y++){
				for(x=0; x<driverinfo.w; x++){
					vt_locate_display(x, y);
					fwrite((char*)(vt[cur_vt].buffer + vt[cur_vt].bufsize - (driverinfo.h*driverinfo.w*2) - vt[cur_vt].scroll * (driverinfo.w*2) + y * driverinfo.w*2 + x*2), 1, 2, driverstream);
					vt_update_display_cursor();
				}
			}
			*/
		}
	}
	else{
		if(vt[cur_vt].buffer){
			memcpy((char*)0xB8000, vt[cur_vt].buffer + vt[cur_vt].bufsize - 160*25,
					160*25);
			vt_fallback_update_cursor();
		}
	}
}

void buffer_add_lines(unsigned int vt_num, unsigned int lines)
{
	if(!initialized) return;
	if(!driverstream) return; //ei bufferia fallbackina
	if (vt_num >= VT_COUNT) return;
	if(vt[cur_vt].buffer){
		memmove(vt[cur_vt].buffer, vt[cur_vt].buffer + (lines * vt[cur_vt].bufw * 2),
				vt[vt_num].bufsize - (lines * vt[vt_num].bufw * 2));
		vt_fill_with_blank(vt[vt_num].buffer + vt[vt_num].bufsize
				- (lines * vt[vt_num].bufw * 2), (lines * vt[vt_num].bufw));
	}
}


void vt_change(unsigned int vt_num)
{
	if(!initialized) return;
	if(!driverstream) return; //ei vt:n vaihtoa fallbackina (ei bufferia)
	if (vt_num >= VT_COUNT) return;
	if (cur_vt == vt_num) return;
	if (spinl_locked(&vt[cur_vt].writelock) || spinl_locked(&vt[cur_vt].printlock)) {
		change_to_vt = vt_num;
		return;
	}
	change_to_vt = 0;
	cur_vt = vt_num;
	update_from_current_buf();
	do_eop();
}

void vt_scroll(int lines) {
	if(!initialized) return;
	if(!driverstream) return; //ei skrollailua fallbackina (ei bufferia)
	/*if(lines < -vt[cur_vt].scroll)
		lines = -vt[cur_vt].scroll;
	if(lines > vt[cur_vt].bufh - vt[cur_vt].scroll)
		lines = vt[cur_vt].bufh - vt[cur_vt].scroll;*/
	vt[cur_vt].scroll += lines;
	if(vt[cur_vt].scroll < 0)
		vt[cur_vt].scroll = 0;
	if(vt[cur_vt].scroll > vt[cur_vt].bufh - driverinfo.h)
		vt[cur_vt].scroll = vt[cur_vt].bufh - driverinfo.h;
	update_from_current_buf();
}

void vt_putch(unsigned int vt_num, int c)
{
	if(!initialized) return;
	if (vt_num >= VT_COUNT) return;
	if (threading_on()) {
		spinl_lock(&vt[vt_num].writelock);
	}

	if (vt[vt_num].scroll != 0) {
		vt[vt_num].scroll = 0;
		update_from_current_buf();
	}
	
	if (c == '\b') { /* backspace */
		if (vt[vt_num].cx > 0) {
			vt[vt_num].cx--;
		} else {
			vt[vt_num].cx = (driverinfo.w) - 1;
			vt[vt_num].cy--;
		}
		vt_update_display_cursor();
	}
	else if (c == '\t') { /* tab */
		vt[vt_num].cx = (vt[vt_num].cx + 8) & ~7;
		vt_update_display_cursor();
	}
	else if (c == '\r') { /* return */
		vt[vt_num].cx = 0;
		vt_update_display_cursor();
	}
	else if (c == '\n') { /* new line */
		vt[vt_num].cx = 0;
		vt[vt_num].cy++;
		vt_update_display_cursor();
	}
	else if (c >= ' ') { /* printable character */
		if(vt_num == cur_vt) {
			if(driverstream){
				char cc[2];
				cc[0] = c;
				cc[1] = vt[vt_num].color;
				fwrite(cc, 1, 2, driverstream);
			}
			else{
				*(char*)(0xB8000 + vt[vt_num].cy * 160 + vt[vt_num].cx * 2) = c;
				*(char*)(0xB8000 + vt[vt_num].cy * 160 + vt[vt_num].cx * 2 + 1) = 0x7;
			}
		}
		if(vt[vt_num].buffer && initialized) {
			*(vt[vt_num].buffer + vt[vt_num].bufsize - (driverinfo.h*driverinfo.w*2) + vt[vt_num].cy * (driverinfo.w*2) + vt[vt_num].cx * 2) = c;
			*(vt[vt_num].buffer + vt[vt_num].bufsize - (driverinfo.h*driverinfo.w*2) + vt[vt_num].cy * (driverinfo.w*2) + vt[vt_num].cx * 2 + 1) = vt[vt_num].color;
		}
		vt[vt_num].cx++;
	}

	
	if(driverstream){
		if (vt[vt_num].cx >= (driverinfo.w)) {
			vt[vt_num].cx = 0;
			vt[vt_num].cy++;
		}
		if (vt[vt_num].cy >= driverinfo.h) { /* scroll screen */
			int amount = vt[vt_num].cy - (driverinfo.h - 1);
			buffer_add_lines(vt_num, amount);
			ioctl(driverstream, IOCTL_DISPLAY_ROLL_UP, amount);
			vt[vt_num].cy = driverinfo.h - 1;
			vt_update_display_cursor();
		}
	}
	else{
		if (vt[vt_num].cx >= 80) {
			vt[vt_num].cx = 0;
			vt[vt_num].cy++;
		}
		if (vt[vt_num].cy >= 25) { /* scroll screen */
			int amount = vt[vt_num].cy - (25 - 1);
			
			memmove((char *)0xB8000, (char *)0xB8000 + amount * 160,
					(25 - amount) * 160);
			vt_fill_with_blank((char *)0xB8000 + (25 - amount) * 160, 80 * amount);

			vt[vt_num].cy = 25 - 1;
		}
	}
	if (threading_on()) {
		spinl_unlock(&vt[vt_num].writelock);
	}
	vt_fallback_update_cursor();
	do_eop();
}

void vt_keyboard(int code, int down)
{
	if(!initialized) return;
	if(!vt[cur_vt].kb_buf) return;
	vt[cur_vt].kb_buf[vt[cur_vt].kb_buf_end] = code | (down ? 0 : 256);
	++vt[cur_vt].kb_buf_count;
	++vt[cur_vt].kb_buf_end;
	vt[cur_vt].kb_buf_end %= KB_BUFFER_SIZE;
}

int vt_kb_peek(unsigned int vt_num)
{
	if(!initialized) return -1;
	if (!vt[vt_num].kb_buf_count) {
		return -1;
	}
	return vt[vt_num].kb_buf[vt[vt_num].kb_buf_start++];
}

int vt_kb_get(unsigned int vt_num)
{
	if(!initialized) return -1;
	int ret;
	extern void taikatemppu();

	while (!vt[vt_num].kb_buf_count) {
		taikatemppu(&vt[vt_num].kb_buf_count);
	}

	ret = vt[vt_num].kb_buf[vt[vt_num].kb_buf_start];
	--vt[vt_num].kb_buf_count;
	++vt[vt_num].kb_buf_start;
	vt[vt_num].kb_buf_start %= KB_BUFFER_SIZE;

	/*if (kb_buf_full) {
		asm_cli();
		while (kb_buf_full) {
			keyboard_handle();
		}
		asm_sti();
		//asm_hlt();
	}*/

	return ret;
}

void vt_kprintflock(unsigned int vt_num)
{
	if(!initialized) return;
	if(threading_on()) {
		spinl_lock(&vt[vt_num].printlock);
		vt[vt_num].in_kprintf = 1;
	}
}

void vt_kprintfunlock(unsigned int vt_num)
{
	if(!initialized) return;
	if(threading_on()) {
		spinl_unlock(&vt[vt_num].printlock);
		vt[vt_num].in_kprintf = 0;
	}
}

int vt_setdriver(char *fname)
{
	int i;
	if(fname==NULL){ //NULL = fallback
		for(i = 0; i < VT_COUNT; i++) {
			if(vt[i].buffer) kfree(vt[i].buffer);
			memset(&vt[i], 0, sizeof(struct vt_t));
			vt[i].scroll = 0;
			vt[i].in_kprintf = 0;
			vt[i].cx = vt[i].cy = 0;
			vt[i].color = 0x7;
			vt[i].buffer = 0;
			vt[i].kb_buf_count = 0;
			vt[i].kb_buf_start = 0;
			vt[i].kb_buf_end = 0;
			spinl_init(&vt[i].writelock);
			spinl_init(&vt[i].printlock);
		}

		cur_vt = 0;
		memset(&driverinfo, 0, sizeof(driverinfo));
		driverstream = NULL;
		return 0;
	}
	
	if(!initialized) return 1;

	driverstream = fopen(fname, "w");
	if(driverstream == NULL){
		kprintf("vt_setdriver(): driverstream=NULL\n");
		return 1;
	}
	memset(&driverinfo, 0, sizeof(driverinfo));
	if(ioctl(driverstream, IOCTL_DISPLAY_GET_INFO, (uintptr_t)&driverinfo)){
		driverstream = NULL;
		kprintf("vt_setdriver(): couldn't get display info\n");
		return 1;
	}
	for(i = 0; i < VT_COUNT; i++) {
		if(vt[i].buffer) kfree(vt[i].buffer);

		vt[i].cx = vt[i].cy = 0;
		vt[i].bufh = VT_BUF_H;
		vt[i].bufw = driverinfo.w;
		vt[i].bufsize = vt[i].bufh * vt[i].bufw * 2;

		vt[i].buffer = (char*)kmalloc(vt[i].bufsize);

		vt_fill_with_blank(vt[i].buffer, driverinfo.h * driverinfo.w);
	}
	
	kprintf("vt_setdriver(): driver set\n");

	return 0;
}

void vt_init(void)
{
	vt_setdriver(NULL);
	initialized = 1;
}


