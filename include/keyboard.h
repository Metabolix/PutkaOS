#ifndef _KEYBOARD_H
#define _KEYBOARD_H

void keyboard_install(void);
unsigned int kb_get(void);

int key_to_ascii(int key, int mods);
int ktoasc(int key);

enum KEYB_MODS {
	KEYB_MOD_RCTRL = 0x01,
	KEYB_MOD_LCTRL = 0x02,
	KEYB_MOD_CTRL = 0x03,
	KEYB_MOD_LSHIFT = 0x04,
	KEYB_MOD_RSHIFT = 0x08,
	KEYB_MOD_SHIFT = KEYB_MOD_LSHIFT | KEYB_MOD_RSHIFT,
	KEYB_MOD_LALT = 0x10,
	KEYB_MOD_ALTGR = 0x20, // Recognize it? :P
	KEYB_MOD_CAPS = 0x100,
	KEYB_MOD_SCRL = 0x200,
	KEYB_MOD_NUML = 0x400,

	KEYB_MOD_UPCASE = KEYB_MOD_CAPS | KEYB_MOD_SHIFT,
};

#define KB_BUFFER_SIZE 128

extern char *nappien_nimet_qwerty[256];
extern unsigned int kb_mods;

enum keys { /* let's put these here if someone needs them... */
KEY__0x00 = 0x00,
	KEY_ESC = 0x01,
	KEY_1 = 0x02,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0, // 0x0b

KEY__0x0c = 0x0c,
KEY__0x0d = 0x0d,
//	KEY_MINUS = 0x0c,
//	KEY_ACCENT = 0x0d,

	KEY_BACKSPACE = 0x0e,
	KEY_TAB = 0x0f,

	KEY_Q = 0x10,
	KEY_W,
	KEY_E,
	KEY_R,
	KEY_T,
	KEY_Y,
	KEY_U,
	KEY_I,
	KEY_O,
	KEY_P, // = 0x19,

KEY__0x1a = 0x1a,
KEY__0x1b = 0x1b,
	KEY_ENTER = 0x1c,

	KEY_LCTRL = 0x1d,
	KEY_RCTRL = KEY_LCTRL | 0x80, // escaped, 0xe0

	KEY_A = 0x1e,
	KEY_S,
	KEY_D,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_J,
	KEY_K,
	KEY_L, // = 0x26,

KEY__0x27 = 0x27,
KEY__0x28 = 0x28,
KEY__0x29 = 0x29,
	KEY_LSHIFT = 0x2a,
KEY__0x2b = 0x2b,

	KEY_Z = 0x2c,
	KEY_X,
	KEY_C,
	KEY_V,
	KEY_B,
	KEY_N,
	KEY_M, // = 0x32,

	KEY_COMMA = 0x33,
	KEY_DOT = 0x34,
KEY__0x35 = 0x35,
	KEY_RSHIFT = 0x36,
	KEY_MUL = 0x37,
	KEY_LALT = 0x38,
	KEY_ALTGR = KEY_LALT | 0x80,

	KEY_SPACE = 0x39,
	KEY_CAPSLOCK = 0x3a,

	KEY_F1 = 0x3b,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10, // = 0x44,

	KEY_NUMLOCK = 0x45,
	KEY_SCROLL_LOCK = 0x46,
	KEY_NUM_7 = 0x47,
	KEY_NUM_8,
	KEY_NUM_9,
	KEY_SUB = 0x4a,
	KEY_NUM_4 = 0x4b,
	KEY_NUM_5,
	KEY_NUM_6,
	KEY_ADD = 0x4e,
	KEY_NUM_1 = 0x4f,
	KEY_NUM_2,
	KEY_NUM_3,
	KEY_NUM_0 = 0x52,
	KEY_POINT = 0x53,

KEY__0x54 = 0x54,
KEY__0x55 = 0x55,
KEY__0x56 = 0x56,
	KEY_F11 = 0x57,
	KEY_F12 = 0x58,

	KEY_NUM_ENTER = KEY_ENTER | 0x80,

	KEY_HOME = KEY_NUM_7 | 0x80,
	KEY_UP = KEY_NUM_8 | 0x80,
	KEY_PGUP = KEY_NUM_9 | 0x80,

	KEY_LEFT = KEY_NUM_4 | 0x80,
	KEY_RIGHT = KEY_NUM_6 | 0x80,

	KEY_END = KEY_NUM_1 | 0x80,
	KEY_DOWN = KEY_NUM_2 | 0x80,
	KEY_PGDOWN = KEY_NUM_3 | 0x80,

};
#endif
