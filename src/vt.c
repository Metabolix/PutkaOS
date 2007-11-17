/*
 * virtuaalitermisjuttu
 * celeron55
 */

#include <multitasking/process.h>
#include <multitasking/multitasking.h>
#include <vt.h>
#include <keyboard.h>
#include <screen.h>
#include <stdio.h>
#include <misc_asm.h>
#include <panic.h>
#include <devices/devmanager.h>

/////////////////////////////////////////////

int initialized = 0;

struct vt_t vt[VT_COUNT];
unsigned int cur_vt;
int change_to_vt;

FILE *driverstream = NULL;
struct displayinfo driverinfo;

int devices_not_removed_count = 0;

struct vt_device_str {
	DEVICE std;
	unsigned int vt_num;
} **vt_devs;

struct vt_file_str {
	FILE std;
	unsigned int vt_num;
};

/////////////////////////////////////////////

void vt_fallback_update_cursor(void)
{
	if(!initialized) return;
	unsigned int temp = vt[cur_vt].cy * 80 + vt[cur_vt].cx;
	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

void vt_fallback_cls(void)
{
	if(!initialized) return;
	int i;
	for(i=0; i<80*25; i++){
		*(char*)(0xB8000 + i * 2 + 0) = ' ';
		*(char*)(0xB8000 + i * 2 + 1) = 0x7;
	}
	vt_fallback_update_cursor();
}

void vt_display_locate(unsigned int x, unsigned int y)
{
	if(!initialized) return;
	if(!driverstream) return;
	int xy[2];
	xy[0] = x;
	xy[1] = y;
	ioctl(driverstream, IOCTL_DISPLAY_LOCATE, (uintptr_t)&xy);
}

void vt_update_cursor()
{
	if(driverstream){
		vt_display_locate(vt[cur_vt].cx, vt[cur_vt].cy + vt[cur_vt].scroll);
	}
	else{
		vt_fallback_update_cursor();
	}
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


void do_eop(void)
{
	if(!initialized) return;
	/* if user wants to change vt while printing we queue it */
	if (change_to_vt) {
		vt_change(change_to_vt);
	}
}

void update_from_current_buf(void)
{
	if(!initialized) return;
	if(driverstream){
		if(vt[cur_vt].buffer){
			vt_display_locate(0, 0);
			fwrite((char*)vt[cur_vt].buffer + vt[cur_vt].bufsize
					- (driverinfo.h * driverinfo.w * 2)
					- (vt[cur_vt].scroll * driverinfo.w * 2), 1,
					(driverinfo.h * driverinfo.w * 2), driverstream);
			vt_update_cursor();

			/*
			int x, y;
			for(y=0; y<driverinfo.h; y++){
				for(x=0; x<driverinfo.w; x++){
					vt_display_locate(x, y);
					fwrite((char*)(vt[cur_vt].buffer + vt[cur_vt].bufsize - (driverinfo.h*driverinfo.w*2) - vt[cur_vt].scroll * (driverinfo.w*2) + y * driverinfo.w*2 + x*2), 1, 2, driverstream);
					vt_update_cursor();
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

void vt_scroll_if_needed(unsigned int vt_num)
{
	if(driverstream){
		if (vt[vt_num].cy >= driverinfo.h) { /* scroll screen */
			unsigned int amount = vt[vt_num].cy - (driverinfo.h - 1);
			if(amount > driverinfo.h) amount = driverinfo.h;
			buffer_add_lines(vt_num, amount);
			ioctl(driverstream, IOCTL_DISPLAY_ROLL_UP, amount);
			vt[vt_num].cy = driverinfo.h - 1;
			vt_update_cursor();
		}
	}
	else{
		if (vt[vt_num].cy >= 25) { /* scroll screen */
			int amount = vt[vt_num].cy - (25 - 1);

			memmove((char *)0xB8000, (char *)0xB8000 + amount * 160,
					(25 - amount) * 160);
			vt_fill_with_blank((char *)0xB8000 + (25 - amount) * 160, 80 * amount);

			vt[vt_num].cy = 25 - 1;
		}
	}
}

//////////////////////////////////////

unsigned int vt_out_get(void)
{
	if(!initialized) return 0;
	if (is_threading()) {
		return active_process->vt_num;
	}

	return VT_KERN_LOG;
}

unsigned char vt_get_color(unsigned int vt_num)
{
	if(!initialized) return -1;
	if (vt_num >= VT_COUNT) return -1;
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

void vt_getdisplaysize(unsigned int vt_num, unsigned int *w, unsigned int *h)
{
	if(!initialized) return;
	if(vt_num >= VT_COUNT) return;
	if(driverstream){
		*w = driverinfo.w;
		*h = driverinfo.h;
	}
	else{
		*w = 80;
		*h = 25;
	}
}

int vt_locate(unsigned int vt_num, unsigned int x, unsigned int y)
{
	if(!initialized) return 1;
	if(vt_num >= VT_COUNT) return 1;
	int ret = 0;
	if(driverstream){
		if(x >= driverinfo.w){
			x = driverinfo.w - 1;
			ret = 1;
		}
		if(y >= driverinfo.h){
			y = driverinfo.h - 1;
			ret = 1;
		}
	}
	else{
		if(x >= 80){
			x = 79;
			ret = 1;
		}
		if(y >= 25){
			y = 24;
			ret = 1;
		}
	}
	vt[vt_num].cx = x;
	vt[vt_num].cy = y;
	if(vt_num == cur_vt){
		vt_update_cursor();
	}

	return ret;
}

int vt_getpos(unsigned int vt_num, unsigned int *x, unsigned int *y)
{
	if(!initialized) return 1;
	if(vt_num >= VT_COUNT) return 1;
	*x = vt[vt_num].cx;
	*y = vt[vt_num].cy;
	return 0;
}

void vt_cls(unsigned int vt_num)
{
	if(!initialized) return;
	if (vt_num >= VT_COUNT) return;
	vt[vt_num].cx = 0;
	vt[vt_num].cy = 0;
	if(vt_num == cur_vt){
		if(driverstream){
			ioctl(driverstream, IOCTL_DISPLAY_CLS, NULL);
		}
		else{
			vt_fallback_cls();
		}
	}
	if(vt[vt_num].buffer){
		vt_fill_with_blank(vt[vt_num].buffer + vt[vt_num].bufsize - driverinfo.h*vt[vt_num].bufw*2, driverinfo.h*vt[vt_num].bufw);
	}
}

unsigned int vt_get_display_height(void)
{
	if(!initialized) return 0;
	if(driverstream) return driverinfo.h;
	else return 25;
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

/* (ei toimi eikä nopeuta)
//- Piirtää nopeasti suoraan merkki-väri-muodossa.
//- len on piirrettävän pituus tavuina.
//- Toimii aioastaan normaalia display-driveriä käytettäessä
int vt_fastprint(unsigned int vt_num, const char *buf, unsigned int len)
{
	if(!initialized) return 1;
	if(!driverstream) return 1;
	if(vt_num >= VT_COUNT) return 1;

	int cx = vt[vt_num].cx, cy = vt[vt_num].cy;
	//int oldcx = cx, oldcy = cy;
	cx += len/2;
	int rowsmore = cx / driverinfo.w;
	if(rowsmore > driverinfo.h) return 1;
	cx -= rowsmore * driverinfo.w;
	cy += rowsmore;
	if(cy > (driverinfo.h - 1)){
		int scrollamount = cy - (driverinfo.h - 1);
		buffer_add_lines(vt_num, scrollamount);
		ioctl(driverstream, IOCTL_DISPLAY_ROLL_UP, scrollamount);
		cy -= scrollamount;
	}
	cy -= rowsmore;
	vt_display_locate(cx, cy);

	if (is_threading()) {
		spinl_lock(&vt[vt_num].writelock);
	}
	fwrite(buf, 1, len, driverstream);
	memcpy(vt[vt_num].buffer + vt[vt_num].bufsize - (driverinfo.h*driverinfo.w*2) + cy * (driverinfo.w*2) + cx * 2, buf, len);

	vt[vt_num].cx = cx;
	vt[vt_num].cy = cy+rowsmore;

	if (is_threading()) {
		spinl_unlock(&vt[vt_num].writelock);
	}

	return 0;
}
*/

//jos zero_ends != 0, tulostetaan niin pitkästi että tulee 0, muuten
//tulostetaan lengthin verran
int vt_print_length(unsigned int vt_num, const char *string,
		unsigned int length, unsigned char zero_ends)
{
	if (!initialized) return -1;
	if (vt_num >= VT_COUNT) return -1;
	if (!vt[vt_num].in_kprintf) {
		if (is_threading()) {
			spinl_lock(&vt[vt_num].printlock);
		}
	}

	char tempbuf[160];
	int i = 0, maxl;
	unsigned int a = 0;
	const char *s = (const char *)string;
	char *s2, *s3;
	while((zero_ends) ? (*s) : (a<length)){
		//kirjoitetaan nopeammin ohjausmerkittömiä pätkiä jos käytetään ajuria
		if(driverstream && *s >= ' '){
			vt_scroll_if_needed(vt_num);
			//vt_putch(vt_num, '|');
			maxl = sizeof(tempbuf);
			if(maxl > vt[vt_num].bufw*2 - vt[vt_num].cx*2)
				maxl = vt[vt_num].bufw*2 - vt[vt_num].cx*2;
			i=0;
			while(*s >= ' ' && ((zero_ends) ? (*s) : (a<length)) && i + 1 < maxl){
				tempbuf[i++] = *s;
				tempbuf[i++] = vt[vt_num].color;
				s++;
				a++;
			}

			if (is_threading()) {
				spinl_lock(&vt[vt_num].writelock);
			}

			//ja bufferiin sama sössö
			s2 = vt[vt_num].buffer + vt[vt_num].bufsize - (driverinfo.h*driverinfo.w*2)
					+ vt[vt_num].cy * (driverinfo.w*2) + vt[vt_num].cx * 2;
			s3 = tempbuf;
			memcpy(s2, s3, i);

			fwrite(tempbuf, 1, i, driverstream);

			vt[vt_num].cx += i/2;
			if(vt[vt_num].cx >= vt[vt_num].bufw){
				vt[vt_num].cx = 0;
				vt[vt_num].cy++;
			}

			if (is_threading()) {
				spinl_unlock(&vt[vt_num].writelock);
			}
		}
		//muuten jumitellaan putchilla
		else{
			vt_putch(vt_num, *s);
			s++;
			a++;
		}
	}

	if (!vt[vt_num].in_kprintf) {
		if (is_threading()) {
			spinl_unlock(&vt[vt_num].printlock);
		}
	}

	return s - string;
}

int vt_print(unsigned int vt_num, const char *string)
{
	return vt_print_length(vt_num, string, 0, 1);
	
	/*
	if (!initialized) return 1;
	if (vt_num >= VT_COUNT) return 1;
	if (!vt[vt_num].in_kprintf) {
		if (is_threading()) {
			spinl_lock(&vt[vt_num].printlock);
		}
	}

	char tempbuf[160];
	int i = 0, maxl;
	const char *s = (const char *)string;
	char *s2, *s3;
	while (*s) {
		//kirjoitetaan nopeammin ohjausmerkittömiä pätkiä jos käytetään ajuria
		if(driverstream && *s >= ' '){
			vt_scroll_if_needed(vt_num);
			//vt_putch(vt_num, '|');
			maxl = sizeof(tempbuf);
			if(maxl > vt[vt_num].bufw*2 - vt[vt_num].cx*2)
				maxl = vt[vt_num].bufw*2 - vt[vt_num].cx*2;
			i=0;
			while(*s >= ' ' && i + 1 < maxl){
				tempbuf[i++] = *s;
				tempbuf[i++] = vt[vt_num].color;
				s++;
			}

			if (is_threading()) {
				spinl_lock(&vt[vt_num].writelock);
			}

			//ja bufferiin sama sössö
			s2 = vt[vt_num].buffer + vt[vt_num].bufsize - (driverinfo.h*driverinfo.w*2)
					+ vt[vt_num].cy * (driverinfo.w*2) + vt[vt_num].cx * 2;
			s3 = tempbuf;
			memcpy(s2, s3, i);

			fwrite(tempbuf, 1, i, driverstream);

			vt[vt_num].cx += i/2;
			if(vt[vt_num].cx >= vt[vt_num].bufw){
				vt[vt_num].cx = 0;
				vt[vt_num].cy++;
			}

			if (is_threading()) {
				spinl_unlock(&vt[vt_num].writelock);
			}
		}
		//muuten jumitellaan putchilla
		else{
			vt_putch(vt_num, *s);
			s++;
		}
	}

	if (!vt[vt_num].in_kprintf) {
		if (is_threading()) {
			spinl_unlock(&vt[vt_num].printlock);
		}
	}

	return s - string;*/
}

int vt_putch(unsigned int vt_num, int c)
{
	if(!initialized) return 1;
	if (vt_num >= VT_COUNT) return 1;
	if (is_threading()) {
		spinl_lock(&vt[vt_num].writelock);
	}

	if (vt[vt_num].scroll != 0) {
		vt[vt_num].scroll = 0;
		vt_scroll(0);
	}

	if (c == '\b') { /* backspace */
		if (vt[vt_num].cx > 0) {
			vt[vt_num].cx--;
		} else {
			vt[vt_num].cx = (driverinfo.w) - 1;
			vt[vt_num].cy--;
		}
		vt_update_cursor();
	}
	else if (c == '\t') { /* tab */
		vt[vt_num].cx = (vt[vt_num].cx + 8) & ~7;
		vt_update_cursor();
	}
	else if (c == '\r') { /* return */
		vt[vt_num].cx = 0;
		vt_update_cursor();
	}
	else if (c == '\n') { /* new line */
		vt[vt_num].cx = 0;
		vt_scroll_if_needed(vt_num);
		vt[vt_num].cy++;
		vt_update_cursor();
	}
	else if (c >= ' ') { /* printable character */
		vt_scroll_if_needed(vt_num);
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
	}
	else{
		if (vt[vt_num].cx >= 80) {
			vt[vt_num].cx = 0;
			vt[vt_num].cy++;
		}
	}

	if (is_threading()) {
		spinl_unlock(&vt[vt_num].writelock);
	}
	do_eop();
	return 0;
}


//####### näppisjuttuja ######


void do_kb_mods(int code, int up, /*struct lockkeystates_str *locks, */uint_t *kb_mods)
{
	switch (code) {
		case KEYCODE_LSHIFT:
			(*kb_mods) |= KEYB_MOD_LSHIFT; if (up) (*kb_mods) ^= KEYB_MOD_LSHIFT;
			break;
		case KEYCODE_RSHIFT:
			(*kb_mods) |= KEYB_MOD_RSHIFT; if (up) (*kb_mods) ^= KEYB_MOD_RSHIFT;
			break;
		case KEYCODE_LCTRL:
			(*kb_mods) |= KEYB_MOD_LCTRL; if (up) (*kb_mods) ^= KEYB_MOD_LCTRL;
			break;
		case KEYCODE_RCTRL:
			(*kb_mods) |= KEYB_MOD_RCTRL; if (up) (*kb_mods) ^= KEYB_MOD_RCTRL;
			break;
		case KEYCODE_LALT:
			(*kb_mods) |= KEYB_MOD_LALT; if (up) (*kb_mods) ^= KEYB_MOD_LALT;
			break;
		case KEYCODE_ALTGR:
			(*kb_mods) |= KEYB_MOD_ALTGR; if (up) (*kb_mods) ^= KEYB_MOD_ALTGR;
			break;
		case KEYCODE_SCROLL_LOCK:
			if(!up) (*kb_mods) ^= KEYB_MOD_SCRL;
			break;
		case KEYCODE_CAPSLOCK:
			if(!up) (*kb_mods) ^= KEYB_MOD_CAPS;
			break;
		case KEYCODE_NUMLOCK:
			if(!up) (*kb_mods) ^= KEYB_MOD_NUML;
			break;
		//default:
	}
}

/*
 * näppisajuri syöttää vt_keyboard_eventille näppäinkoodin ja tiedon
 * näppäimen suunnasta.
 * 
 * vt:n säilömä keyeventti koostuu keycodesta ja 0x100:n kohdalla olevasta
 * bitistä joka ilmaisee onko nappula mennyt ylös vai alas (1=ylös).
 */
void vt_keyboard_event(int code, int up)
{
	if(!initialized) return;
	if(!vt[cur_vt].kb_buf) return; //(näin ei taida kyllä edes voida käydä)

	code &= 0xff;

	//kprintf("\n[event,c=%i,u=%i]", code, up);
	
	//spinl_lock(&vt[cur_vt].kb_buf_lock);
	
	switch(code){
		case KEYCODE_PGUP:
			if ((vt[cur_vt].kb_mods & KEYB_MOD_SHIFT)){
				if(!up) vt_scroll(vt_get_display_height()/2);
				return;
			}
		case KEYCODE_PGDOWN:
			if ((vt[cur_vt].kb_mods & KEYB_MOD_SHIFT)){
				//vt_scroll(-1);
				if(!up) vt_scroll(-(int)vt_get_display_height()/2);
				return;
			}
		default:
			if (KEYCODE_F1 <= code && code <= KEYCODE_F6) { // f1-f6
				if(!up) vt_change(code - KEYCODE_F1);
				return;
			}
			else if ((code == KEYCODE_C) && (vt[cur_vt].realtime_kb_mods & KEYB_MOD_CTRL)) {
#if 0
				if(!up){
					extern tid_t sh_tid;
					kill_thread(sh_tid);
					vt_unlockspinlocks();
					sh_tid = new_thread(run_sh, 0, 0, 0);
				}
				return;
#endif
			}
	}

	do_kb_mods(code, up, /*&vt[cur_vt].realtime_kb_locks,*/ &vt[cur_vt].realtime_kb_mods);

	//kprintf(" [[[%s]]] ",(vt[cur_vt].kb_mods&(KEYB_MOD_LSHIFT|KEYB_MOD_RSHIFT))?"s":"n");

	vt[cur_vt].kb_buf[vt[cur_vt].kb_buf_end] = code | (up ? 0x100 : 0);
	++vt[cur_vt].kb_buf_count;
	++vt[cur_vt].kb_buf_end;
	vt[cur_vt].kb_buf_end %= KB_BUFFER_SIZE;
	//spinl_unlock(&vt[cur_vt].kb_buf_lock);
}

int vt_get_and_parse_next_key_event(unsigned int vt_num)
{
	//static unsigned char capsl_key, numl_key, scroll_key;

	if(!initialized) return -3;
	if (vt_num >= VT_COUNT) return -3;
	if (!vt[vt_num].kb_buf_count) {
		return -1;
	}

	//spinl_lock(&vt[vt_num].kb_buf_lock);
	
	unsigned int keyevent = vt[vt_num].kb_buf[vt[vt_num].kb_buf_start];
	--vt[vt_num].kb_buf_count;
	++vt[vt_num].kb_buf_start;
	vt[vt_num].kb_buf_start %= KB_BUFFER_SIZE;
	unsigned char code = keyevent & 0xff;
	unsigned char up = (keyevent & 0x100)?1:0;

	do_kb_mods(code, up, /*&vt[cur_vt].kb_locks,*/ &vt[cur_vt].kb_mods);

	//kprintf(" [[%s]] ",(vt[vt_num].kb_mods&(KEYB_MOD_LSHIFT|KEYB_MOD_RSHIFT))?"s":"n");

	//spinl_unlock(&vt[vt_num].kb_buf_lock);

	return keyevent;
}

//-1 jos ei ole
int vt_get_next_key_event(unsigned int vt_num)
{
	return vt_get_and_parse_next_key_event(vt_num);
}

int vt_wait_and_get_next_key_event(unsigned int vt_num)
{
	while (!vt[vt_num].kb_buf_count) {
		switch_thread();
	}
	return vt_get_and_parse_next_key_event(vt_num);
}

int vt_get_kbmods(unsigned int vt_num)
{
	return vt[vt_num].kb_mods;
}

//palauttaa -1 jos ei löydetty bufferista merkkejä ja muita <0-lukuja muina
//virheinä
int vt_get_next_char(unsigned int vt_num)
{
	if(!initialized) return -5;
	if (vt_num >= VT_COUNT) return -5;
	for(;;){
		int keyevent = vt_get_and_parse_next_key_event(vt_num);
		if(keyevent<0){
			//print("[<0]");
			return keyevent;
		}
		//kprintf("[%x]", keyevent);
		unsigned char code = keyevent & 0xff;
		unsigned char up = (keyevent & 0x100)?1:0;
		if(code == KEYCODE_LSHIFT || code == KEYCODE_RSHIFT
				|| code == KEYCODE_LCTRL || code == KEYCODE_RCTRL
				|| code == KEYCODE_LALT || code == KEYCODE_ALTGR
				|| code == KEYCODE_SCROLL_LOCK
				|| code == KEYCODE_CAPSLOCK
				|| code == KEYCODE_NUMLOCK) continue;
		//kprintf("[c=%i|u=%i]", code, up);
		if(!up){
			//kprintf(" [%s] ",(vt[vt_num].kb_mods&(KEYB_MOD_LSHIFT|KEYB_MOD_RSHIFT))?"s":"n");
			//kprintf(" [[%s]] ",(vt[vt_num].kb_mods&(KEYB_MOD_CAPS))?"c":"n");
			int c = key_to_ascii(code, vt[vt_num].kb_mods);
			/*kprintf("[%x]", c);
			while(1);*/
			return c;
		}
	}
	return -6;
}

//palauttaa -2 jos ei ole initialisoitu ja -1 jos bufferi on tyhjä
int vt_kb_peek(unsigned int vt_num)
{
	return vt_get_next_char(vt_num);
	/*if(!initialized) return -1;
	if (!vt[vt_num].kb_buf_count) {
		return -1;
	}
	return vt[vt_num].kb_buf[vt[vt_num].kb_buf_start++];*/
}

int vt_kb_get(unsigned int vt_num)
{
	if(!initialized) return -1;
	int c;
	for(;;){
		while (!vt[vt_num].kb_buf_count) {
			switch_thread();
		}
		//print(" count>0 ");
		c = vt_get_next_char(vt_num);
		if(c==-1){
			//print("(-1)\n");
			//continue; //oli joku turha event
		}
		else if(c<-1){
			//panic("vt_get_next_char returned < -1");
		}
		else{
			//print("\n");
			return c;
		}
	}
	//kprintf("%i", c);
	
	//return c;
	
	/*int ret;

	while (!vt[vt_num].kb_buf_count) {
		asm_hlt(&vt[vt_num].kb_buf_count);
	}

	ret = vt[vt_num].kb_buf[vt[vt_num].kb_buf_start];
	--vt[vt_num].kb_buf_count;
	++vt[vt_num].kb_buf_start;
	vt[vt_num].kb_buf_start %= KB_BUFFER_SIZE;*/

	/*if (kb_buf_full) {
		asm_cli();
		while (kb_buf_full) {
			keyboard_handle();
		}
		asm_sti();
		//asm_hlt();
	}*/

	//return ret;
}

void vt_kprintflock(unsigned int vt_num)
{
	if(!initialized) return;
	if(is_threading()) {
		spinl_lock(&vt[vt_num].printlock);
		vt[vt_num].in_kprintf = 1;
	}
}

void vt_kprintfunlock(unsigned int vt_num)
{
	if(!initialized) return;
	if(is_threading()) {
		spinl_unlock(&vt[vt_num].printlock);
		vt[vt_num].in_kprintf = 0;
	}
}

void vt_unlockspinlocks(void)
{
	int i;
	for(i=0; i<VT_COUNT; i++){
		spinl_unlock(&vt[i].writelock);
		spinl_unlock(&vt[i].printlock);
	}
}

int vt_setdriver(char *fname)
{
	int i;
	if(fname==NULL){ //NULL = fallback
		for(i = 0; i < VT_COUNT; i++) {
			if(vt[i].buffer) kfree(vt[i].buffer);
			memset(&vt[i], 0, sizeof(struct vt_t));
			vt[i].color = 0x7;
			spinl_init(&vt[i].writelock);
			spinl_init(&vt[i].printlock);
			//spinl_init(&vt[i].kb_buf_lock);
		}

		cur_vt = 0;
		memset(&driverinfo, 0, sizeof(driverinfo));
		driverstream = NULL;
		return 0;
	}

	if(!initialized) return 1;

	int was_in_fallback_mode = (driverstream == NULL);

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

		vt[i].bufh = VT_BUF_H;
		vt[i].bufw = driverinfo.w;
		vt[i].bufsize = vt[i].bufh * vt[i].bufw * 2;

		vt[i].buffer = (char*)kmalloc(vt[i].bufsize);

		vt_fill_with_blank(vt[i].buffer, driverinfo.h * driverinfo.w);

	}
	if(was_in_fallback_mode && vt[0].buffer){
		memcpy(vt[0].buffer + vt[0].bufsize - driverinfo.w*2 * driverinfo.h,
				(char *)0xB8000, 160*25);
		update_from_current_buf();
		kprintf("vt_setdriver(): was in fallback mode: copied old display contents to buffer\n");
	}

	kprintf("vt_setdriver(): driver set\n");

	return 0;
}

void vt_init(void)
{
	vt_setdriver(NULL);
	initialized = 1;
}


/////////////////////////7


size_t vt_fwrite(const void *buf, size_t size, size_t count,
		struct vt_file_str *vt_file)
{
	int i;
	if((i = vt_print_length(vt_file->vt_num, buf, size*count, 0)) == -1)
		return 0;
	return i/size;
}

size_t vt_fread(void *buf, size_t size, size_t count,
		struct vt_file_str *vt_file)
{
	switch(vt[vt_file->vt_num].mode){
	case VT_MODE_NORMAL:
		for(unsigned int i=0;i<size*count;){
			int ch;
			if(vt[vt_file->vt_num].block)
				ch = vt_kb_get(vt_file->vt_num);
			else
				ch = vt_kb_peek(vt_file->vt_num);
			if(ch < 0) return i/size;
			if(ch<0xff && ch>=0){
				((char*)buf)[i] = (char)ch;
				i++;
			}
			else{
				//jännämerkkejä, TODO: jotain ansisössöä ehkä pitäisi antaa
			}
		}
		break;
	case VT_MODE_RAWEVENTS:
		for(unsigned int i=0;i<size*count/4;){
			int event = vt_get_and_parse_next_key_event(vt_file->vt_num);
			if(event < -1){
				print("vt_fread(): error in vt_get_and_parse_next_key_event\n");
				return i*sizeof(uint_t)/size;
			}
			else if(event == -1){
				if(vt[vt_file->vt_num].block){
					while(!vt[vt_file->vt_num].kb_buf_count) switch_thread();
					continue;
				}
				else return i*sizeof(uint_t)/size;
			}
			((uint_t*)buf)[i] = event;
			i++;
		}
		break;
	default:
		panic("vt_fread(): ei näin");
	}
	return count;
}

int vt_ioctl(struct vt_file_str *vt_file, int request, uintptr_t param)
{
	if(request == IOCTL_VT_MODE){
		if(param>1) return 1;
		vt[vt_file->vt_num].mode = param;
		return 0;
	}
	if(request == IOCTL_VT_BLOCKMODE){
		if(param>1) return 1;
		vt[vt_file->vt_num].block = param;
		return 0;
	}
	return 1;
}

int vt_fclose(struct vt_file_str *vt_file)
{
	if(vt[vt_file->vt_num].num_open==0) return 1;
	if(vt_file){
		if(vt_file->std.func) kfree((void*)vt_file->std.func);
		kfree(vt_file);
	}
	vt[vt_file->vt_num].num_open--;
	kprintf("vt_fclose(): closed (vt_num=%d, num_open=%d\n",
			vt_file->vt_num, vt[vt_file->vt_num].num_open);
	return 0;
}

FILE *vt_open(struct vt_device_str *device, uint_t mode)
{
	if( !( (mode & FILE_MODE_WRITE) || (mode & FILE_MODE_READ) ) ){
		kprintf("vt_open(): only read and/or write supported\n");
		return NULL;
	}
	struct vt_file_str *vt_file =
			(struct vt_file_str*)kmalloc(sizeof(struct vt_file_str));
	if(vt_file == NULL){
		kprintf("vt_open(): kmalloc failed (1)\n");
		return NULL;
	}
	memset(vt_file, 0, sizeof(struct filefunc));
	vt_file->std.mode = mode;
	struct filefunc *func;
	func = (struct filefunc*)kmalloc(sizeof(struct filefunc));
	if(func == NULL){
		kfree(vt_file);
		kprintf("vt_open(): kmalloc failed (2)\n");
		return NULL;
	}
	memset(func, 0, sizeof(struct filefunc));

	func->fwrite = (fwrite_t)&vt_fwrite;
	func->fread = (fread_t)&vt_fread;
	func->ioctl = (ioctl_t)&vt_ioctl;
	func->fclose = (fclose_t)&vt_fclose;

	vt_file->std.func = func;
	
	vt_file->vt_num = device->vt_num;

	vt[device->vt_num].num_open++;

	kprintf("vt_open(): opened\n");

	return (FILE*)vt_file;
}

int vt_remove(struct vt_device_str *device)
{
	if(devices_not_removed_count==0) return 0;
	devices_not_removed_count--;
	if(devices_not_removed_count==0){
		for(unsigned int i=0; i<VT_COUNT; i++){
			kfree(vt_devs[i]);
		}
		kfree(vt_devs);
	}
	return 0;
}

void vt_dev_init(void)
{
	if(!initialized) vt_init();
	
	if(VT_COUNT > 99){
		kprintf("vt_init(): error: too many vts requested!\n");
		return;
	}
	//vt_device_str **
	vt_devs = (struct vt_device_str**)
			kcalloc(sizeof(struct vt_device_str*) * VT_COUNT, 1);
	if(vt_devs==NULL){
		kprintf("vt_init(): error: could not kcalloc (1)\n");
		return;
	}
	for(unsigned int i=0; i<VT_COUNT; i++){
		vt_devs[i] = kcalloc(sizeof(struct vt_device_str), 1);
		if(vt_devs[i]==NULL){
			kprintf("vt_init(): warning: could not kcalloc (2)\n");
			continue;
		}
		char *name = (char*)kmalloc(sizeof(char)*5);
		if(name == NULL){
			kprintf("vt_init(): warning: could not kmalloc (3)\n");
			kfree(vt_devs[i]);
			continue;
		}
		sprintf(name, "vt%i", i);
		vt_devs[i]->std.name = name;
		vt_devs[i]->std.dev_class = DEV_CLASS_OTHER;
		vt_devs[i]->std.dev_type = DEV_TYPE_VT;
		vt_devs[i]->std.devopen = (devopen_t) vt_open;
		vt_devs[i]->std.remove = (devrm_t) vt_remove;
		vt_devs[i]->vt_num = i;
		kprintf("vt_init(): adding %s\n", name);
		switch (device_insert(&vt_devs[i]->std)) {
			case 0:
				devices_not_removed_count++;
				break;
			case DEV_ERR_TOTAL_FAILURE:
				kprintf("vt_init(): unknown error inserting device %d\n", i);
				break;
			case DEV_ERR_BAD_NAME:
				kprintf("vt_init(): error inserting device %d: bad name\n", i);
				break;
			case DEV_ERR_EXISTS:
				kprintf("vt_init(): error inserting device %d: device exists\n", i);
				break;
			case DEV_ERR_BAD_STRUCT:
				kprintf("vt_init(): error inserting device %d: bad info struct\n", i);
				break;
			}
	}
}
