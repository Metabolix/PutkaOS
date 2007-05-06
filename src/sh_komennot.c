#include <sh.h>
#include <screen.h>
#include <panic.h>
#include <timer.h>
#include <io.h>
#include <filesys/mount.h>
#include <string.h>

#include <sh_komennot.h>
void sh_hexcat(char *name);

struct sh_komento komentotaulu[] = {
	{"?", "Apua", sh_help},
	{"help", "Apua", sh_help},
	{"exit", "Paniikki", sh_exit},
	{"panic", "Paniikki", sh_exit},
	{"uptime", "Uptime", sh_uptime},
	{"lscolours", "Listaa varit", sh_list_colours},
	{"colour", "Aseta vari", sh_set_colour},
	{"reset", "Tyhjenna ruutu ja aseta perusvari", sh_reset},
	{"keynames", "Tulostetaan nappien nimia, kunnes tulee Escape", sh_key_names},
	{"outp", "outb port byte, laheta tavu porttiin (dec: 123, hex: 0x7b, oct: 0173)", sh_outportb},
	{"inp", "inp port, hae tavu portista (dec: 123, hex: 0x7b, oct: 0173)", sh_inportb},
	{"history", "Komentohistoria", sh_history},
	{"ls", "ls polku; listaa hakemisto", sh_ls},
	{"cat", "cat polku; tulosta tiedoston sisalto", sh_cat},
	{"hexcat", "hexcat polku; tulosta tiedoston sisalto tavuittain heksana", sh_hexcat},
	{"mountro", "mountro laite liitospiste; liita laite pisteeseen (vain luku)", sh_mount_ro},
	{"remountro", "remountro laite liitospiste; korvaa vanha liitos pisteessa (vain luku)", sh_remount_ro},
	{"mount", "mount laite liitospiste; liita laite pisteeseen", sh_mount},
	{"remount", "remount laite liitospiste; korvaa vanha liitos pisteessa", sh_remount},
	{"umount", "umount {laite | polku}; poista laite tai liitoskohta", sh_umount},
	{"reboot", "reboot; kaynnista tietokone uudelleen", sh_reboot},
	{0, 0, sh_ei_tunnistettu} /* Terminaattori */
};
struct sh_komento *komennot = komentotaulu;

int sh_read_int(char **bufptr)
{
	char *buf = *bufptr;
	int retval = 0;
	if (buf[0] == '0') {
		if (buf[1] == 'x') {
			++buf;
			while (++buf) if (*buf <= '9' && *buf >= '0') {
				retval = 16 * retval + *buf - '0';
			} else if (*buf <= 'f' && *buf >= 'a') {
				retval = 16 * retval + 10 + *buf - 'a';
			} else if (*buf <= 'F' && *buf >= 'A') {
				retval = 16 * retval + 10 + *buf - 'A';
			} else {
				break;
			}
		} else {
			while (++buf) if (*buf <= '7' && *buf >= '0') {
				retval = 8 * retval + *buf - '0';
			} else {
				break;
			}
		}
	} else {
		while (*buf && (*buf <= '9' && *buf >= '0')) {
			retval = 10 * retval + *buf - '0';
			++buf;
		}
	}
	*bufptr = buf;
	return retval;
}

void sh_mount(char *dev_point)
{
	sh_mount_real(dev_point, FILE_MODE_READ | FILE_MODE_WRITE, 0);
}

void sh_mount_ro(char *dev_point)
{
	sh_mount_real(dev_point, FILE_MODE_READ, 0);
}

void sh_remount(char *dev_point)
{
	sh_mount_real(dev_point, FILE_MODE_READ | FILE_MODE_WRITE, 1);
}

void sh_remount_ro(char *dev_point)
{
	sh_mount_real(dev_point, FILE_MODE_READ, 1);
}

void sh_mount_real(char *dev_point, uint_t mode, int remount)
{
	char *dev = dev_point, *point = strchr(dev_point, ' ');
	if (!point) {
		return;
	}
	*point = 0;
	++point;
	switch ((!remount ? mount_something(dev, point, mode) : mount_replace(dev, point, mode))) {
		case 0:
			break;
		case MOUNT_ERR_TOTAL_FAILURE:
			kprintf("sh: mount: Jokin virhe.\n");
			break;
		case MOUNT_ERR_ALREADY_MOUNTED:
			kprintf("sh: mount: Piste on jo liitetty.\n");
			break;
		case MOUNT_ERR_DEVICE_ERROR:
			kprintf("sh: mount: Laitetta ei saada auki.\n");
			break;
		case MOUNT_ERR_FILESYS_ERROR:
			kprintf("sh: mount: Tunnistamaton tiedostojarjestelma.\n");
			break;
		default:
			kprintf("sh: mount: Muu virhe.\n");
			break;
	}
}

void sh_umount(char *dev_point)
{
	switch (umount_something(dev_point)) {
		case MOUNT_ERR_TOTAL_FAILURE:
			kprintf("sh: mount: Jokin virhe.\n");
			break;
		case MOUNT_ERR_MOUNTED_SUBPOINTS:
			kprintf("sh: mount: Alempia liitospisteita on yha olemassa.\n");
			break;
		case MOUNT_ERR_BUSY:
			kprintf("sh: mount: Piste on kiireinen.\n");
			break;
	}
}

void sh_ls(char *name)
{
	DIR *d;
	d = dopen(name);
	if (!d) {
		kprintf("sh: ls: Hakemistoa '%s' ei ole tai ei saada auki.\n", name);
		return;
	}
	while (dread(d) == 0) {
		kprintf("%12s size='%d' created='%d' modified='%d'\n", d->name, d->size, d->created, d->modified);
	}
	dclose(d);
}

void sh_cat(char *name)
{
	FILE *f;
	char buf[257];
	fpos_t pos;
	kprintf("cat '%s'\n", name);
	f = fopen(name, "r");
	if (!f) {
		kprintf("sh: ls: Tiedostoa '%s' ei ole tai ei saada auki.\n", name);
		return;
	}
	buf[256] = 0;
	while (1) {
		if (fgetpos(f, &pos)) {
			kprintf("sh: ls: fgetpos!\n");
			break;
		}
		if (fread(buf, 256, 1, f)) {
			print(buf);
		} else {
			if (fsetpos(f, &pos)) {
				kprintf("sh: ls: fsetpos!\n");
				break;
			}
			buf[fread(buf, 1, 256, f)] = 0;
			print(buf);
			break;
		}
	}
	putch('\n');
	fclose(f);
}

void sh_hexcat(char *name)
{
	int i, j, l;
	FILE *f;
	char buf[257];
	fpos_t pos;
	kprintf("hexcat '%s'\n", name);
	f = fopen(name, "r");
	if (!f) {
		kprintf("sh: ls: Tiedostoa '%s' ei ole tai ei saada auki.\n", name);
		return;
	}
	l = 256;
	while (1) {
		if (fgetpos(f, &pos)) {
			kprintf("sh: ls: fgetpos!\n");
			break;
		}
		print("\n");
		if (fread(buf, 256, 1, f)) {
			for (j = 0; j < 256; j += i) {
				for (i = 0; i < 16; ++i) {
					kprintf("%02x ", (int)(unsigned char)buf[j+i]);
				}
				print("\n");
			}
		} else {
			if (fsetpos(f, &pos)) {
				kprintf("sh: ls: fsetpos!\n");
				break;
			}
			l = fread(buf, 1, 256, f);
			for (j = 0; j < l; j += i) {
				for (i = 0; i < 16 && j+i < l; ++i) {
					kprintf("%02x ", (int)(unsigned char)buf[j+i]);
				}
				print("\n");
			}
			break;
		}
		kwait(0, 500000);
	}
	putch('\n');
	fclose(f);
}

void sh_history(char *buf)
{
	int i;
	for (i = 0; i < SH_HISTORY_SIZE; i++) {
		if (history[i][0] == 0) {
			return;
		}
		kprintf("%s\n", history[i]);
	}
}

void sh_key_names(char *buf)
{
	int ch;
	while ((ch = kb_get())) {
		if (ch == KEY_ESC) {
			break;
		}
		kprintf("(mods: %#06x), %#04x - '%s' (%s)\n", kb_mods, ch & 255, nappien_nimet_qwerty[ch & 255], (ch & 256) ? "up" : "down");
	}
}

void sh_help(char *buf)
{
	komennot = komentotaulu;
	while (komennot->komento) {
		kprintf("%10s -- %s\n", komennot->komento, komennot->kuvaus);
		++komennot;
	}
	komennot = komentotaulu;
}

void sh_reset(char *buf)
{
	cls();
	sh_colour = 7;
}

void sh_set_colour(char *buf)
{
	int colour;
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	if (!*buf) return;
	colour = sh_read_int(&buf);
	if (colour < 0 || colour > 255) return;
	sh_colour = colour;
}

void sh_list_colours(char *buf)
{
	int i, j;
	set_colour(7);
	print("    ");
	for (j = 0; j < 16; ++j) {
		kprintf("x%02x ", j);
	}
	putch('\n');
	for (i = 0; i < 256; i += 16) {
		kprintf("x%02x ", i);
		for (j = 0; j < 16; ++j) {
			set_colour(i+j);
			kprintf("x%02x ", i+j);
		}
		set_colour(7);
		putch('\n');
	}
}

void sh_exit(char *buf)
{
	unsigned char vari[256] = {0};
	vari[' '] = 0;
	vari['.'] = 0x44;
	vari['$'] = 0xdd;
	vari['+'] = 0x99;
	static const char * dont_panic_xpm[] = {
	"  ++                 +        ++                                    ...   ",
	"+++++++     ++++     ++     + ++ ++++++++                         .....   ",
	"++    ++   ++  ++   +++    ++ ++ ++++++++++                 .     ..      ",
	"++     +   ++  ++   +++    +   +     ++                 .   .    ..       ",
	"++     ++ ++   ++  ++ ++  ++        +++           .     .  ..    .        ",
	"++     ++ ++   ++ ++  ++  +         ++           ...    .   ..  ..        ",
	"++     +  +    ++ ++   + ++         ++   ..       ..    .   ..  ..        ",
	"++    ++  +    +  ++   +++          ++   ...      ...   .   ..   ..     ..",
	"++   ++   ++++++ ++    +++       ...$+   .. .     .. .  .    .    .. .... ",
	"++ +++     ++++  ++     +      .....$$.  .   .     .  ....   ..    .....  ",
	"+++++                        ...    +..  .   ..    .   ...   ..           ",
	"                             ..      ..  .   ...   ..   ..    .           ",
	"                             ..      ..  .  .....  ..    .    .           ",
	"                              ..      .  ...    .. ..    .                ",
	"                              ..    ..   .       .  .                     ",
	"                               ......    .        .                       ",
	"                               ....      .                                ",
	"                                ..       .                                ",
	"                                ..                                        ",
	"                                 .                                        ",
	"                                 ..                                       ",
	"                                  .                                       ",
	0};
	const char **rivi, *merkki;
	putch('\n');
	set_colour(7);
	for (rivi = dont_panic_xpm; *rivi; ++rivi) {
		print("  ");
		for (merkki = *rivi; *merkki; ++merkki) {
			set_colour(vari[(unsigned char)*merkki]);
			putch(' ');
			while (merkki[0] == merkki[1]) {
				putch(' ');
				++merkki;
			}
		}
		print("\n");
		set_colour(7);
	}
	putch('\n');
	panic("exit kutsuttu!");
}

void sh_uptime(char *buf)
{
	struct tm sys_time;
	struct timeval uptime;
	get_sys_time(&sys_time);
	get_uptime(&uptime);
	kprintf("On %u.%u. vuonna %u ja kello on %02u.%02u.%02u; uptime %u,%06u sekuntia.\n",
		sys_time.tm_mday, sys_time.tm_mon + 1, sys_time.tm_year + 1900,
		sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec,
		uptime.sec, uptime.usec);
}

void sh_outportb(char *buf)
{
	unsigned int port = 0, byte = 0;
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	port = sh_read_int(&buf);
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	byte = sh_read_int(&buf);

	kprintf("Port %d (%#x), sending %d (%#04x)\n", port, port, byte, byte);
	outportb(port, byte);
}

void sh_inportb(char *buf)
{
	unsigned int port = 0, byte;
	while (*buf && !(*buf <= '9' && *buf >= '0')) ++buf;
	port = sh_read_int(&buf);
	byte = inportb(port);
	kprintf("Port %d (%#x): got %d (%#04x)\n", port, port, byte, byte);
}

void sh_reboot(char *buf)
{
	outportb(0x64, 0x60);
	kwait(0, 500);
	outportb(0x60, 0x14);
	kwait(0, 500);
	outportb(0x64, 0xfe);
}

void sh_ei_tunnistettu(char *buf)
{
	kprintf("Tunnistamaton komento (%s) Mutta 'help' auttaa!\n", buf);
}
