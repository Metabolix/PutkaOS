#ifndef _KEYBOARD_H
#define _KEYBOARD_H
void keyboard_install();
unsigned int kb_get();
unsigned char key_to_ascii(unsigned char key, unsigned char shift, unsigned char control, unsigned char alt);
unsigned char ktoasc(unsigned char key);

#define kb_buffer_size 100

enum keys { /* let's put these here if someone needs them... */
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
	
	KEY_PGUP = 0x49,
	KEY_PGDOWN = 0x51,

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
#endif
