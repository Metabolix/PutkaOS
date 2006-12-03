#include <io.h>
#include <irq.h>
#include <idt.h>
#include <screen.h>
#include <panic.h>

/*
enum napit_qwerty {
	KEY_ESC = 0x01,
	KEY_BACKSPACE = 0x0e,
	KEY_TAB = 0x0f,
	KEY_CAPSLOCK = 0x3a,

	KEY_LSHIFT = 0x2a,
	KEY_RSHIFT = 0x36,
	KEY_LCTRL = 0x1d,
	KEY_RCTRL = 0x1d, // ja 0xe0
	KEY_LALT = 0x38,
	KEY_ENTER = 0x1c,

	KEY_DOT = 0x34,
	KEY_COMMA = 0x33,
	KEY_MINUS = 0x0c,

	KEY_1 = 0x02,
	KEY_2 = 0x03,
	KEY_3 = 0x04,
	KEY_4 = 0x05,
	KEY_5 = 0x06,
	KEY_6 = 0x07,
	KEY_7 = 0x08,
	KEY_8 = 0x09,
	KEY_9 = 0x0a,
	KEY_0 = 0x0b,

	KEY_Q = 0x10,
	KEY_W = 0x11,
	KEY_E = 0x12,
	KEY_R = 0x13,
	KEY_T = 0x14,
	KEY_Y = 0x15,
	KEY_U = 0x16,
	KEY_I = 0x17,
	KEY_O = 0x18,
	KEY_P = 0x19,

	KEY_A = 0x1e,
	KEY_S = 0x1f,
	KEY_D = 0x20,
	KEY_F = 0x21,
	KEY_G = 0x22,
	KEY_H = 0x23,
	KEY_J = 0x24,
	KEY_K = 0x25,
	KEY_L = 0x26,

	KEY_Z = 0x2c,
	KEY_X = 0x2d,
	KEY_C = 0x2e,
	KEY_V = 0x2f,
	KEY_B = 0x30,
	KEY_N = 0x31,
	KEY_M = 0x32,
};
*/

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
void keyboard_handle()
{
	static unsigned char escape, oli_escape, code, up;

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
		//kprintf("DVORAK %s, QWERTY %s (%#04x) %s\n", nappien_nimet_dvorak[code], nappien_nimet_qwerty[code], code & 0x7f, (up ? "up" : "down"));
		kprintf("Keyboard: %s (%#04x) %s\n", nappien_nimet_qwerty[code], code & 0x7f, (up ? "up" : "down"));
	}
}

void keyboard_install() {
	install_irq_handler(1, (void *) keyboard_handle);
	inportb(0x60);
	print("Keyboard installed\n");
}

