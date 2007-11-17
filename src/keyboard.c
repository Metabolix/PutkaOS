#include <keyboard.h>
#include <io.h>
#include <irq.h>
#include <idt.h>
#include <screen.h>
#include <panic.h>
#include <sh.h>
#include <multitasking/multitasking.h>
#include <string.h>
#include <bit.h>
#include <vt.h>

char *nappien_nimet_qwerty[256] = {
	"0x00",
	"Esc",
	"1","2","3","4","5","6","7","8","9","0","0x0c","0x0d","Backspace",
	"Tab","Q","W","E","R","T","Y","U","I","O","P","0x1a","0x1b","Enter", "Left Ctrl",
	"A","S","D","F","G","H","J","K","L","0x27","0x28","0x29",
	"Left Shift", "0x2b", "Z","X","C","V","B","N","M", ",", ".", "0x35", "Right Shift",
	"NP[*]","Left Alt","Space","Caps Lock",
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10",
	"Num Lock", "Scroll Lock",
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
	"Alt Gr", "0xb9", "0xba", "0xbb", "0xbc", "0xbd", "0xbe", "0xbf",
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
	'-', '`','\b',
	'\t','q','w','e','r','t','y','u','i','o','p',0,'"','\n', 0,
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

const char shifted_keys_to_ascii[256] = {
	0,
	27, /* esc */
	'!','"','#',0,'%','&','/','(',')','=',
	'_', '`','\b',
	'\t','Q','W','E','R','T','Y','U','I','O','P',0,'^','\n', 0,
	'A','S','D','F','G','H','J','K','L',0,'*', 0,
	0, 0, 'Z','X','C','V','B','N','M', ';', ':', 0,0,
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

const char altgred_keys_to_ascii[256] = {
	0,
	27, /* esc */
	0,'@',0,'$',0,0,'{','[',']','}',
	'-', '\'','\b',
	'\t','q','w','e','r','t','y','u','i','o','p',0,'"','\n', 0,
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

int key_to_ascii(int key, int kb_mods)
{
	int ret = keys_to_ascii[key&0xff];
	if (ret >= 'a' && ret <= 'z') {
		//if (kb_mods & KEYB_MOD_UPCASE) {
		if (((kb_mods & KEYB_MOD_SHIFT) && !(kb_mods & KEYB_MOD_CAPS))
				|| (!(kb_mods & KEYB_MOD_SHIFT) && (kb_mods & KEYB_MOD_CAPS))) {
			return ret + 'A' - 'a';
		}
		return ret;
	}
	if (kb_mods & KEYB_MOD_SHIFT) {
		ret = shifted_keys_to_ascii[key&0xff];
	} else if (kb_mods & KEYB_MOD_ALTGR || kb_mods & KEYB_MOD_LALT) { /* TODO: Alt pois. :P */
		ret = altgred_keys_to_ascii[key&0xff];
	}
	if(ret==0){
		ret = key<<8;
	}
	return ret;
}

unsigned char *x86_modstatus = (unsigned char *)0x0417;
enum x86_MOD_MASK {
	x86_MOD_DERSHIFT = 0x01,
	x86_MOD_DELSHIFT = 0x02,
	x86_MOD_DECTRL   = 0x04,
	x86_MOD_DEALT    = 0x08,
	x86_MOD_SCRL     = 0x10,
	x86_MOD_NUML     = 0x20,
	x86_MOD_CAPSL    = 0x40,
	x86_MOD_INSMODE  = 0x80,
};

/*int ktoasc(int key) {
	return key_to_ascii(key, kb_mods);
}*/

void keyboard_handle(void)
{
	static unsigned char escaped, oli_escaped, up;
	//static unsigned char capsl_key, numl_key, scroll_key;
	static unsigned int code;

	if (escaped) {
		--escaped;
	} else {
		oli_escaped = 0;
	}
	code = inportb(0x60);
	if (code == 0xe0) {
		escaped = 1;
		oli_escaped = 0x80;
	} else if (code == 0xe1) {
		escaped = 2;
		oli_escaped = 0x80;
	} else if (!escaped) {
		up = (code & 0x80) ? 1 : 0;
		code = (code & 0x7f) | oli_escaped;

		vt_keyboard_event(code, up);
		
		/*switch (code) {
			case KEYCODE_LSHIFT:
				kb_mods |= KEYB_MOD_LSHIFT; if (!down) kb_mods ^= KEYB_MOD_LSHIFT;
				break;
			case KEYCODE_RSHIFT:
				kb_mods |= KEYB_MOD_RSHIFT; if (!down) kb_mods ^= KEYB_MOD_RSHIFT;
				break;
			case KEYCODE_LCTRL:
				kb_mods |= KEYB_MOD_LCTRL; if (!down) kb_mods ^= KEYB_MOD_LCTRL;
				break;
			case KEYCODE_RCTRL:
				kb_mods |= KEYB_MOD_RCTRL; if (!down) kb_mods ^= KEYB_MOD_RCTRL;
				break;
			case KEYCODE_LALT:
				kb_mods |= KEYB_MOD_LALT; if (!down) kb_mods ^= KEYB_MOD_LALT;
				break;
			case KEYCODE_ALTGR:
				kb_mods |= KEYB_MOD_ALTGR; if (!down) kb_mods ^= KEYB_MOD_ALTGR;
				break;
			case KEYCODE_SCROLL_LOCK:
				// Vaihdetaan tila
				if (down && !scroll_key) {
					kb_mods ^= KEYB_MOD_SCRL;
				}
				scroll_key = down;
				break;
			case KEYCODE_CAPSLOCK:
				if (down) return;
				if (capsl_key) {
					capsl_key = 0;
				} else {
					down = capsl_key = 1;
					kb_mods ^= KEYB_MOD_CAPS;
				}
				break;
			case KEYCODE_NUMLOCK:
				if (down) return;
				if (numl_key) {
					numl_key = 0;
				} else {
					down = numl_key = 1;
					kb_mods ^= KEYB_MOD_NUML;
				}
				break;
			case KEYCODE_PGUP:
				if ((kb_mods & KEYB_MOD_SHIFT) && down)
					vt_scroll(vt_get_display_height()/2);
				break;
			case KEYCODE_PGDOWN:
				if ((kb_mods & KEYB_MOD_SHIFT) && down)
					//vt_scroll(-1);
					vt_scroll(-(int)vt_get_display_height()/2);
				break;
			default:
				if (KEYCODE_F1 <= code && code <= KEYCODE_F6) { // f1-f6
					vt_change(code - KEYCODE_F1);
				} else if ((code == KEYCODE_C) && (kb_mods & KEYB_MOD_CTRL) && down) {
#if 0
					extern tid_t sh_tid;
					kill_thread(sh_tid);
					vt_unlockspinlocks();
					sh_tid = new_thread(run_sh, 0, 0, 0);
#endif
					break;
				}
		}*/
	}
}

void keyboard_install(void)
{
	install_irq_handler(1, keyboard_handle);
	inportb(0x60); /* There might be something in the buffer */
	*x86_modstatus &= ~(x86_MOD_SCRL | x86_MOD_NUML | x86_MOD_CAPSL);
	*x86_modstatus |= x86_MOD_INSMODE;
	print("Keyboard installed\n");
}

