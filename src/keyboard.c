#include <io.h>
#include <irq.h>
#include <idt.h>
#include <screen.h>
#include <panic.h>
#include <mem.h>

char *nappien_nimet_qwerty[256] = {
	"0x00",
	"Esc",
	"1","2","3","4","5","6","7","8","9","0","-","`","Backspace",
	"Tab","Q","W","E","R","T","Y","U","I","O","P","&aring;","^","Enter", "Left Ctrl",
	"A","S","D","F","G","H","J","K","L","0x27","'", "0x28",
	"Left Shift", "0x2b", "Z","X","C","V","B","N","M", ",", ".", "0x35", "Right Shift",
	"NP[*]","Left Alt","Space","Caps Lock",
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10",
	"Num Lock","0x46",
	"NP[7]","NP[8]","NP[9]", "NP[-]",
	"NP[4]","NP[5]","NP[6]", "NP[+]",
	"NP[1]","NP[2]","NP[3]","NP[0]","NP[Del]",
	"0x54", "0x55", "0x56",
	"F11", "F12",
	"0x59", "0x5a", "0x5b", "0x5c", "0x5d", "0x5e", "0x5f",
	"0x60", "0x61", "0x62", "0x63", "0x64", "0x65", "0x66", "0x67", "0x68", "0x69", "0x6a", "0x6b",
	"0x6c", "0x6d", "0x6e", "0x6f", "0x70", "0x71", "0x72", "0x73", "0x74", "0x75", "0x76", "0x77",
	"0x78", "0x79", "0x7a", "0x7b", "0x7c", "0x7d", "0x7e", "0x7f", "0x80", "0x81", "0x82", "0x83",
	"0x84", "0x85", "0x86", "0x87", "0x88", "0x89", "0x8a", "0x8b", "0x8c", "0x8d", "0x8e", "0x8f",
	"0x90", "0x91", "0x92", "0x93", "0x94", "0x95", "0x96", "0x97", "0x98", "0x99", "0x9a", "0x9b",
	"NP[Enter]",
	"Right Ctrl",
	"0x9e", "0x9f", "0xa0", "0xa1", "0xa2", "0xa3", "0xa4", "0xa5", "0xa6", "0xa7",
	"0xa8", "0xa9", "0xaa", "0xab", "0xac", "0xad", "0xae", "0xaf", "0xb0", "0xb1", "0xb2", "0xb3",
	"0xb4",
	"NP[/]",
	"0xb6",
	"Print Scrn / SysRq",
	"0xb8", "0xb9", "0xba", "0xbb", "0xbc", "0xbd", "0xbe", "0xbf",
	"0xc0", "0xc1", "0xc2", "0xc3", "0xc4",
	"0xc5",
	"Pause/Break",
	"Home", "Up Arrow", "Page Up",
	"0xca",
	"Left Arrow", "0xcc", "Right Arrow",
	"0xce",
	"End", "Down Arrow", "Page Down", "Insert", "Delete",
	"0xd4", "0xd5", "0xd6", "0xd7",
	"0xd8", "0xd9", "0xda",
	"Left WinKey", "Right WinKey", "Menu Key",
	"0xde", "0xdf", "0xe0", "0xe1", "0xe2", "0xe3",
	"0xe4", "0xe5", "0xe6", "0xe7", "0xe8", "0xe9", "0xea", "0xeb", "0xec", "0xed", "0xee", "0xef",
	"0xf0", "0xf1", "0xf2", "0xf3", "0xf4", "0xf5", "0xf6", "0xf7", "0xf8", "0xf9", "0xfa", "0xfb",
	"0xfc", "0xfd", "0xfe", "0xff"
};
/*
char *nappien_nimet_dvorak[256] = {
	"0x00",
	"Esc",
	"1","2","3","4","5","6","7","8","9","0","Z","`","Backspace",
	"Tab","'",",",".","P","Y","F","G","C","R","L","/?","=","Enter", "Left Ctrl",
	"A","O","E","U","I","D","H","T","N","S","-", "0x28",
	"Left Shift", "0x2b", ";:","Q","J","K","X","B","M", "W", "V", "0x35", "Right Shift",
	"NP[*]","Left Alt","Space","Caps Lock",
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10",
	"Num Lock","0x46",
	"NP[7]","NP[8]","NP[9]", "NP[-]",
	"NP[4]","NP[5]","NP[6]", "NP[+]",
	"NP[1]","NP[2]","NP[3]","NP[0]","NP[Del]",
	"0x54", "0x55", "0x56",
	"F11", "F12",
	"0x59", "0x5a", "0x5b", "0x5c", "0x5d", "0x5e", "0x5f",
	"0x60", "0x61", "0x62", "0x63", "0x64", "0x65", "0x66", "0x67", "0x68", "0x69", "0x6a", "0x6b",
	"0x6c", "0x6d", "0x6e", "0x6f", "0x70", "0x71", "0x72", "0x73", "0x74", "0x75", "0x76", "0x77",
	"0x78", "0x79", "0x7a", "0x7b", "0x7c", "0x7d", "0x7e", "0x7f", "0x80", "0x81", "0x82", "0x83",
	"0x84", "0x85", "0x86", "0x87", "0x88", "0x89", "0x8a", "0x8b", "0x8c", "0x8d", "0x8e", "0x8f",
	"0x90", "0x91", "0x92", "0x93", "0x94", "0x95", "0x96", "0x97", "0x98", "0x99", "0x9a", "0x9b",
	"NP[Enter]",
	"Right Ctrl",
	"0x9e", "0x9f", "0xa0", "0xa1", "0xa2", "0xa3", "0xa4", "0xa5", "0xa6", "0xa7",
	"0xa8", "0xa9", "0xaa", "0xab", "0xac", "0xad", "0xae", "0xaf", "0xb0", "0xb1", "0xb2", "0xb3",
	"0xb4",
	"NP[/]",
	"0xb6",
	"Print Scrn / SysRq",
	"0xb8", "0xb9", "0xba", "0xbb", "0xbc", "0xbd", "0xbe", "0xbf",
	"0xc0", "0xc1", "0xc2", "0xc3", "0xc4",
	"0xc5",
	"Pause/Break",
	"Home", "Up Arrow", "Page Up",
	"0xca",
	"Left Arrow", "0xcc", "Right Arrow",
	"0xce",
	"End", "Down Arrow", "Page Down", "Insert", "Delete",
	"0xd4", "0xd5", "0xd6", "0xd7",
	"0xd8", "0xd9", "0xda",
	"Left WinKey", "Right WinKey", "Menu Key",
	"0xde", "0xdf", "0xe0", "0xe1", "0xe2", "0xe3",
	"0xe4", "0xe5", "0xe6", "0xe7", "0xe8", "0xe9", "0xea", "0xeb", "0xec", "0xed", "0xee", "0xef",
	"0xf0", "0xf1", "0xf2", "0xf3", "0xf4", "0xf5", "0xf6", "0xf7", "0xf8", "0xf9", "0xfa", "0xfb",
	"0xfc", "0xfd", "0xfe", "0xff"
};
*/

const char keys_to_ascii[256] = {
	0,
	27, /* esc */
	'1','2','3','4','5','6','7','8','9','0',
	'-', '`','\b',				      /* left control */
	'\t','q','w','e','r','t','y','u','i','o','p',0,'^','\n', 0,
	'a','s','d','f','g','h','j','k','l',0,'\'', 0,
	0, 0, 'z','x','c','v','b','n','m', ',', '.', 0,0,
	'*',0,' ',0,
	/*"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10",*/
	0,0,0,0,0,0,0,0,0,0,
	0,0,
	'7','8','9','-','4','5','6','+','1','2','3','0',127,
	0,0,0,
	0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	'\n',
	0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,
	47,
	0,
	0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,
	0,
	0,0,0,
	0,
	0,0,0,
	0,
	0,0,0,0,0,
	0,0,0,0,
	0,0,0,
	0,0,0,
	0,0,0,0,0,0,
	0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0
};




unsigned char key_to_ascii(unsigned char key, unsigned char shift, unsigned char control, unsigned char alt) {
	unsigned char ret = keys_to_ascii[key];
	if(ret >= 'a' && ret <= 'z' && shift)
		ret += 'A' - 'a';
	if(ret == '+' && shift)
		ret = '?';
	if(ret == '-' && shift)
		ret = '_'; /* and so on... we should make a table */
	return ret;
}

unsigned char shift = 0, control = 0, alt = 0;
unsigned char kb_buf_full = 0;

unsigned char ktoasc(unsigned char key) {
	return key_to_ascii(key, shift, control, alt);
}


void keyboard_handle(void)
{
	static unsigned char escape, oli_escape, up;
	static unsigned int code;

	/*print("KBD: interrupt\n");
	kprintf("%d %d %d\n", vt[cur_vt].kb_buff_filled, kb_buffer_size, kb_buf_full);*/

	if (vt[cur_vt].kb_buff_filled == kb_buffer_size) { /* our buffer is full */
		kprintf("buffer is full!\n");
		kb_buf_full = 1;
		return;
	}

	kb_buf_full = 0;

	if (escape) {
		--escape;
	} else {
		oli_escape = 0;
	}
	code = inportb(0x60);
	if (code == 0xe0) {
		escape = 1;
		oli_escape = 0x80;
	} else if (code == 0xe1) {
		escape = 2;
		oli_escape = 0x80;
	} else if (!escape) {
		up = (code & 0x80) ? 1 : 0;
		code = (code & 0x7f) | oli_escape;

		switch (code & 0x7f){
			case 0x2a: /* left shift or right shift */
			case 0x36:
				shift =  !up;
				break;
			case 0x1d: /* control */
				control = !up;
				break;
			case 0x38: /* left alt */
				alt = !up;
				break;
			default:
				if(code >= 0x3b && code <= 0x3f) { /* f1-f6 */
					change_vt(code - 0x3b);
				}
		}

		if(vt[cur_vt].kb_buff_filled == kb_buffer_size) {
			memmove((void*)vt[cur_vt].kb_buff, (void*)(vt[cur_vt].kb_buff + 1), kb_buffer_size);
			vt[cur_vt].kb_buff_filled--;
		}
		vt[cur_vt].kb_buff[vt[cur_vt].kb_buff_filled++] = code | (up?0:256);

		//kprintf("DVORAK %s, QWERTY %s (%#04x) %s\n", nappien_nimet_dvorak[code], nappien_nimet_qwerty[code], code & 0x7f, (up ? "up" : "down"));
		//kprintf("Keyboard: %s (%#04x) %s\n", nappien_nimet_qwerty[code], code & 0x7f, (up ? "up" : "down"));
	}
}

unsigned int kb_get(void) {
	unsigned int ret;
	unsigned int curr_vt = cur_vt;
 	while(!vt[curr_vt].kb_buff_filled) {
	}

	ret = vt[curr_vt].kb_buff[0];
	memcpy((void*)(vt[curr_vt].kb_buff), (void*)(vt[curr_vt].kb_buff + 1), --vt[curr_vt].kb_buff_filled);

	if(kb_buf_full > 0) {
		keyboard_handle();
	}

	return ret;
}

void keyboard_install(void) {
	install_irq_handler(1, (void *) keyboard_handle);
	inportb(0x60); /* There might be something in the buffer */
	print("Keyboard installed\n");
}

