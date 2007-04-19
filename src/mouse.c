#include <mouse.h>
#include <io.h>
#include <string.h>
#include <screen.h>
#include <irq.h>

int mouse_id;
struct mouse_state mouse;

static int mouse_wait_data(void)
{
	int time_out = 100000;
	while (time_out--) {
		if ((inportb(0x64) & 1) == 1) {
			return 0;
		}
	}
	return -1;
}
static int mouse_wait_signal(void)
{
	int time_out = 100000;
	while (time_out--) {
		if ((inportb(0x64) & 2) == 0) {
			return 0;
		}
	}
	return -1;
}

static void mouse_write(unsigned char b)
{
	mouse_wait_signal();
	outportb(0x64, 0xD4); // We're talking to the mouse, not keyboard...
	mouse_wait_signal();
	outportb(0x60, b);
}

static unsigned char mouse_read(void)
{
	mouse_wait_data();
	return inportb(0x60);
}

static unsigned char mouse_cmd(unsigned char b)
{
	outportb(0x64, b);
}

static int mouse_get_9bit(int sign, int most)
{
	union convert_9bit {
		signed i :9;
		struct {unsigned most :8, sign :1;};
	} conv;
	conv.most = most;
	conv.sign = sign;
	return conv.i;
}

void mouse_get_state(struct mouse_state *state)
{
	if (!state) {
		return;
	}
	*state = mouse;
	memset(&mouse, 0, offsetof(struct mouse_state, empty));
}

void mouse_handle(void)
{
	struct mouse_movement_data data;
	print("Moi.\n");
	mouse_cmd(0xAD);
	data.char1 = mouse_read();
	data.char2 = mouse_read();
	data.char3 = mouse_read();
	data.char4 = (mouse_id ? mouse_read() : 0);
	mouse_cmd(0xAE);
	mouse.dx += mouse_get_9bit(data.x_sign, data.x_most);
	mouse.dy += mouse_get_9bit(data.y_sign, data.y_most);
	mouse.dz += data.z;
#define BTN(a) \
	if ((mouse.btn_##a != 0) != (data.btn_##a != 0)) { \
		if (data.btn_##a) { \
			mouse.click_##a++; \
		} \
		mouse.btn_##a = data.btn_##a; \
	}
	BTN(l); BTN(m); BTN(r); BTN(4); BTN(5);
}

#define GET_ACK if (mouse_read() != MOUSE_ANS_ACKNOWLEDGE) return -1;
int mouse_go_super(void)
{
	mouse_write(MOUSE_CMD_SET_SAMPLE_RATE); GET_ACK
	mouse_write(200); GET_ACK
	mouse_write(MOUSE_CMD_SET_SAMPLE_RATE); GET_ACK
	mouse_write(200); GET_ACK
	mouse_write(MOUSE_CMD_SET_SAMPLE_RATE); GET_ACK
	mouse_write(80); GET_ACK
	mouse_write(MOUSE_CMD_GET_ID); GET_ACK
	mouse_id = mouse_read();
	if (mouse_id == 4) return 0;
	if (mouse_id != 0) return -1;

	mouse_write(MOUSE_CMD_SET_SAMPLE_RATE); GET_ACK
	mouse_write(200); GET_ACK
	mouse_write(MOUSE_CMD_SET_SAMPLE_RATE); GET_ACK
	mouse_write(100); GET_ACK
	mouse_write(MOUSE_CMD_SET_SAMPLE_RATE); GET_ACK
	mouse_write(80); GET_ACK
	mouse_write(MOUSE_CMD_GET_ID); GET_ACK
	mouse_id = mouse_read();
	if (mouse_id == 3) return 0;
	if (mouse_id != 0) return -1;
	return 1;
}
#undef GET_ACK

void mouse_install(void)
{
	int skippaa_super = 0;
alku:
	memset(&mouse, 0, offsetof(struct mouse_state, empty));
	mouse_write(MOUSE_CMD_RESET);
	if ((mouse_read() != MOUSE_ANS_ACKNOWLEDGE)
	|| (mouse_read() != MOUSE_ANS_SELF_TEST_PASSED)) {
		kprintf("No PS/2 mouse found...\n", mouse_id);
		return;
	}
	mouse_id = mouse_read();
	if (!skippaa_super) {
		if (mouse_go_super() < 0) {
			skippaa_super = 1;
			goto alku;
		}
	}
	mouse_write(MOUSE_CMD_SET_DEFAULTS);
	if (mouse_read() != MOUSE_ANS_ACKNOWLEDGE) {
		return;
	}
	mouse_write(MOUSE_CMD_SET_MODE_STREAM);
	if (mouse_read() != MOUSE_ANS_ACKNOWLEDGE) {
		return;
	}
	mouse_cmd(0xA8);
	mouse_cmd(0x20);
	unsigned char status = mouse_read();
	mouse_cmd(0x60);
	mouse_write(status | 2);
	mouse_cmd(0xd4);
	mouse_write(MOUSE_CMD_ENABLE);
	if (mouse_read() != MOUSE_ANS_ACKNOWLEDGE) {
		return;
	}
	install_irq_handler(12, (void *) mouse_handle);
	kprintf("Mouse installed; id = %d\n", mouse_id);
}
