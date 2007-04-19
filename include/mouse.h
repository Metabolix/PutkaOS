#ifndef _MOUSE_H_
#define _MOUSE_H_ 1

enum MOUSE_COMMANDS {
	MOUSE_CMD_RESET = 0xff,
	MOUSE_CMD_RESEND = 0xfe,
	MOUSE_CMD_SET_DEFAULTS = 0xf6,
	MOUSE_CMD_DISABLE = 0xf5,
	MOUSE_CMD_ENABLE = 0xf4,
	MOUSE_CMD_SET_SAMPLE_RATE = 0xf3,
	MOUSE_CMD_GET_ID = 0xf2,
	MOUSE_CMD_SET_MODE_REMOTE = 0xf0,
	MOUSE_CMD_SET_MODE_WARP = 0xee,
	MOUSE_CMD_SET_MODE_STREAM = 0xea,
	MOUSE_CMD_UNSET_MODE_WARP = 0xec,
	MOUSE_CMD_READ_DATA = 0xeb,
	MOUSE_CMD_STATUS_REQUEST = 0xf9,
	MOUSE_CMD_SET_RESOLUTION = 0xf8,
	MOUSE_CMD_SET_SCALING_1 = 0xf7,
	MOUSE_CMD_SET_SCALING_2 = 0xf6,
};

enum MOUSE_ANSWERS {
	MOUSE_ANS_ACKNOWLEDGE = 0xfa,
	MOUSE_ANS_SELF_TEST_PASSED = 0xaa,
};

struct mouse_movement_data {
	union {
		struct {
			signed char
				btn_l :1, btn_r :1, btn_m :1,
				reserved_a :1,
				x_sign :1, y_sign :1,
				x_overflow :1, y_overflow :1;
		};
		unsigned char char1;
	};
	union { unsigned char x_most; unsigned char char2; };
	union { unsigned char y_most; unsigned char char3; };
	union {
		struct {
			signed char
				z :4,
				btn_4 :1, btn_5 :1,
				reserved_b :2;
		};
		unsigned char char4;
	};
};

struct mouse_status_data {
	union {
		struct {
			signed char
				btn_r :1, btn_m :1, btn_l :1,
				reserved_a :1,
				scaling :1, enabled :1, is_remote :1,
				reserved_b :1;
		};
		unsigned char char1;
	};
	unsigned char resolution, sample_rate;
};

struct mouse_state {
	int dx, dy, dz;
	int click_l, click_m, click_r, click_4, click_5;
	union {
		int empty;
		struct {int btn_l :1, btn_m :1, btn_r :1, btn_4 :1, btn_5 :1;};
	};
};

extern void mouse_get_state(struct mouse_state *state);
extern void mouse_install(void);

#endif
