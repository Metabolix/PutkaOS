#include <devices/blockdev/floppy.h>
#include <devices/blockdev/blockdev.h>
#include <string.h>
#include <io.h>
#include <irq.h>
#include <screen.h>
#include <bit.h>
#include <timer.h>
#include <panic.h>
#include <misc_asm.h>
#include <stdint.h>

/*
 * Internals
**/
static void floppy_handler(void);

static void floppy_wait_data(void);
static void floppy_configure(void);
static void floppy_command(uint8_t command);
static void floppy_wait(void);
static void floppy_sense_interrupt(void);
static void floppy_calibrate(uint_t drive);
static void floppy_init_dma(uintptr_t buffer, size_t len, int write);
static void floppy_motor_on(uint_t drive);
static void floppy_prepare_motor_off(uint_t drive);
static void floppy_motor_off(uint_t drive);
static int floppy_seek_track(uint_t drive, uint_t track);
static void floppy_reset_flipflop_dma(void);

int floppy_rw_sector(uint_t drive, uint8_t sector, uint8_t head, uint8_t cylinder, uintptr_t buffer, int write);
int floppy_read_one_block(struct floppy *self, uint64_t num64, void * buf);
int floppy_write_one_block(struct floppy *self, uint64_t num64, const void * buf);

struct floppy_parameters * const floppy_params = (struct floppy_parameters *) DISK_PARAMETER_ADDRESS;
int cylinder, status_0;
#define wait_irq(a) kwait(0, 20*1000)

struct floppy floppy_drives[2];
char floppy_buffer[512];
unsigned int floppy_drive_seeked = 0;
unsigned int floppy_motors = 0;

#define FLOPPY_MOTOR_OFF(x) floppy_motor_off_ ## x
#define FLOPPY_MOTOR_OFF_FUNC(x) void FLOPPY_MOTOR_OFF(x) (void) {floppy_motor_off(x);}
FLOPPY_MOTOR_OFF_FUNC(0)
FLOPPY_MOTOR_OFF_FUNC(1)
void (*floppy_motor_off_x[])(void) = {
	FLOPPY_MOTOR_OFF(0),
	FLOPPY_MOTOR_OFF(1)
};

const char floppy_unknown[] = "unknown floppy drive";
const char * floppy_drive_types[16] = {
	"no floppy drive",
	"360KB 5.25\" floppy drive",
	"1.2MB 5.25\" floppy drive",
	"720KB 3.5\" floppy drive",
	"1.44MB 3.5\" floppy drive",
	"2.88MB 3.5\" floppy drive",
	floppy_unknown, floppy_unknown,
	floppy_unknown, floppy_unknown, floppy_unknown, floppy_unknown,
	floppy_unknown, floppy_unknown, floppy_unknown, floppy_unknown
};

struct floppy floppy_drives[2] = {
	{
		{
			{
				"fd0",
				DEV_CLASS_BLOCK,
				DEV_TYPE_FLOPPY,
				-1, // devmanager sets
				(devopen_t) blockdev_fopen,
				(devrm_t) 0, // TODO: safely remove... :P
			},

			0, // uint64_t block_size; Filled in install_floppy
			0, // uint64_t block_count; Filled in install_floppy
			0, // uint64_t first_block_num;

			(read_one_block_t) floppy_read_one_block,
			(write_one_block_t) floppy_write_one_block,
			(read_blocks_t) 0,
			(write_blocks_t) 0
		},
		0, 0, 0, 0, 0, 0
	},
	{
		{
			{
				"fd1",
				DEV_CLASS_BLOCK,
				DEV_TYPE_FLOPPY,
				-1, // devmanager sets
				(devopen_t) blockdev_fopen,
				(devrm_t) 0, // TODO: safely remove... :P
			},

			0, // uint64_t block_size; Filled in install_floppy
			0, // uint64_t block_count; Filled in install_floppy
			0, // uint64_t first_block_num;

			(read_one_block_t) floppy_read_one_block,
			(write_one_block_t) floppy_write_one_block,
			(read_blocks_t) 0,
			(write_blocks_t) 0
		},
		1, 0, 0, 0, 0, 0
	}
};

void floppy_handler(void) { }

void floppy_init(void)
{
	int i;
	uint8_t detect_floppy;

	//memcpy(&floppy_params, (void*)DISK_PARAMETER_ADDRESS, sizeof(struct floppy_parameters));

	if (floppy_params->bytes_per_sector > 2) {
		print("FDD: ERROR: Sector size bigger than 512 bytes (disabled floppies)\n");
		floppy_drives[0].type = 0;
		floppy_drives[1].type = 0;
		return;
	}

	outportb(0x70, 0x10);
	detect_floppy = inportb(0x71);

	floppy_drives[0].type = detect_floppy >> 4;
	floppy_drives[1].type = detect_floppy & 0xF;

	for (i = 0; i < MAX_DRIVES; ++i) {
		if (floppy_drives[i].type == 4) {
			device_insert(&floppy_drives[i].blockdev.std);

			/* with 1.44MB 3.5" floppy disk, 2 sides and 80 tracks per side */
			floppy_drives[i].blockdev.block_size = 128 * (1 << floppy_params->bytes_per_sector);
			floppy_drives[i].blockdev.block_count = 2 * 80 * floppy_params->sectors_per_track;
		} else {
			if (floppy_drives[i].type) {
				kprintf("FDD: Found %s at %s, that's not supported.\n",
					floppy_drive_types[floppy_drives[i].type],
					floppy_drives[i].type);
			}
			floppy_drives[i].blockdev.std.dev_type = DEV_TYPE_NONE;
		}
	}

	install_irq_handler(6, (irq_handler_t)floppy_handler);
}

void floppy_reset(void)
{
	int a;

	prepare_wait_irq(FLOPPY_IRQ);
	outportb((FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER), 0x00); /*disable controller*/
	kwait(0, 1000 * 50);
	outportb((FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER), 0x0c); /*enable controller*/

	print("FDD: Reseted controller\n");
	wait_irq(FLOPPY_IRQ);
	print("FDD: Waited for it\n");

	for(a = 0; a < 4; a++) {
		floppy_sense_interrupt();
	}

	outportb(FLOPPY_FIRST + CONFIGURATION_CONTROL_REGISTER, 0);

	floppy_configure();

	if (floppy_drives[0].type) {
		floppy_calibrate(0);
	}
	if (floppy_drives[1].type) {
		floppy_calibrate(1);
	}
	print("FDD: Calibrated drives\n");
}

void floppy_wait_data(void)
{
	/* While RQM and DIO aren't 1 */
	while ((inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0xd0) != 0xd0);
}


void floppy_configure(void)
{
	floppy_command(SPECIFY);
	floppy_command(floppy_params->steprate_headunload);
	floppy_command(floppy_params->headload_ndma & 254);
	print("FDD: Drive configured\n");
}

void floppy_command(uint8_t command)
{
	floppy_wait();
	outportb(FLOPPY_FIRST + DATA_FIFO, command);
}

void floppy_wait(void)
{
	while (!(inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x80));
}

void floppy_sense_interrupt(void)
{
	floppy_command(SENSE_INTERRUPT);
	floppy_wait_data();
	floppy_drives[floppy_drive_seeked].status = inportb(FLOPPY_FIRST + DATA_FIFO);
	floppy_wait_data();
	floppy_drives[floppy_drive_seeked].track = inportb(FLOPPY_FIRST + DATA_FIFO);
}

void floppy_calibrate(uint_t drive)
{
	if (drive >= MAX_DRIVES || !floppy_drives[drive].type) {
		return;
	}

	prepare_wait_irq(FLOPPY_IRQ);
	floppy_command(RECALIBRATE); /* (re)calibrate drive*/
	floppy_command(drive);
	wait_irq(FLOPPY_IRQ);
	kprintf("FDD: fd%u calibrated\n", drive);
}

void floppy_init_dma(uintptr_t buffer, size_t len, int write)
{
	asm_cli();
	outportb(0x0a, 0x06);      /* mask DMA channel 2 */
	floppy_reset_flipflop_dma();
	outportb(0x4, buffer & 0xFF);
	outportb(0x4, (buffer >> 8) & 0xFF);
	floppy_reset_flipflop_dma();
	len--;
	outportb(0x5, len & 0xFF);
	outportb(0x5, len >> 8);
	outportb(0x81, buffer >> 16);
	outportb(0x0b, (write ? 0x48: 0x44) + 2);  /* single transfer, write or read, channel 2 */
	outportb(0x0a, 0x02);          /* unmask DMA channel 2 */
	asm_sti();
}

void floppy_motor_on(uint_t drive)
{
	if (drive >= MAX_DRIVES || floppy_drives[drive].motor) {
		return;
	}
	if (floppy_drives[drive].motor_off_timer) {
		ktimer_stop(floppy_drives[drive].motor_off_timer);
		floppy_drives[drive].motor_off_timer = 0;
	}
	floppy_drives[drive].motor = 1;
	floppy_motors |= (1 << drive);
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC + (floppy_motors << 4)); /* motor on */
	kwait(0, 1000 * 500); /* wait it spins up */
}

void floppy_prepare_motor_off(uint_t drive)
{
	if (drive >= MAX_DRIVES) {
		return;
	}
	if (floppy_drives[drive].motor_off_timer) {
		ktimer_stop(floppy_drives[drive].motor_off_timer);
		floppy_drives[drive].motor_off_timer = 0;
	}
	floppy_drives[drive].motor_off_timer = ktimer_start(floppy_motor_off_x[drive], 500000, 1);
}

void floppy_motor_off(uint_t drive)
{
	if (drive >= MAX_DRIVES || floppy_drives[drive].motor == 0) {
		return;
	}
	floppy_motors &= ~(1 << drive);
	floppy_drives[drive].motor = 0;
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC | floppy_motors << 4);
	//kprintf("FDD: Motor %u off\n", drive);
}

int floppy_seek_track(uint_t drive, uint_t track)
{
	if (drive >= MAX_DRIVES) {
		return -1;
	}

	if (track == floppy_drives[drive].track) {
		return 0;
	}

	floppy_motor_on(drive);

	prepare_wait_irq(FLOPPY_IRQ);
	floppy_command(SEEK);
	floppy_command(drive);
	floppy_command(track);
	wait_irq(FLOPPY_IRQ);

	floppy_drive_seeked = drive;

	floppy_sense_interrupt();
	if (track != floppy_drives[drive].track) {
		kprintf("FDD: ERROR: fd%u couldn't seek\n", drive);
		floppy_prepare_motor_off(drive);
		return -2;
	}
	/*kprintf("FDD: fd%u: Seeked track succesfully\n", drive);*/
	floppy_prepare_motor_off(drive);
	return 0;
}

void floppy_reset_flipflop_dma(void)
{
	outportb(0xC, 0);
}

int floppy_rw_sector(uint_t drive, uint8_t sector, uint8_t head, uint8_t cylinder, uintptr_t buffer, int write)
{
	int a;
alku:
	if (drive >= MAX_DRIVES || !floppy_drives[drive].type) {
		return -1;
	}

	if (inportb(FLOPPY_FIRST + DIGITAL_INPUT_REGISTER) & 0x80) {
		/* disk was changed */
		floppy_seek_track(drive, 1);
		floppy_calibrate(drive);
		//floppy_motor_off(drive);
		if (inportb(FLOPPY_FIRST + DIGITAL_INPUT_REGISTER) & 0x80) {
			kprintf("FDD: No floppy in fd%u\n", drive);
			return -2;
		} else {
			print("FDD: Floppy changed, trying again...\n");
			goto alku;
		}
	}

	if (floppy_seek_track(drive, cylinder))
	if (floppy_seek_track(drive, cylinder))
	if (floppy_seek_track(drive, cylinder)) {
		return -1; /* three seeks? */
	}

	/* floppy_seek_track actually starts motor already, but... */
	floppy_motor_on(drive);

	if (!inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x20) {
		panic("Non-dma floppy transfer?\n");
	}

	/* block size */
	floppy_init_dma(buffer, 512, write);

	kwait(0, 1000 * floppy_params->head_settle_time);
	floppy_wait();

	prepare_wait_irq(FLOPPY_IRQ);
	floppy_command(write ? WRITE_DATA : READ_DATA);
	floppy_command((head << 2) | drive);
	floppy_command(cylinder);
	floppy_command(head);
	floppy_command(sector);
	floppy_command(floppy_params->bytes_per_sector);  /*sector size = 128*2^size*/
	floppy_command(floppy_params->sectors_per_track); /*last sector*/
	floppy_command(floppy_params->gap_length);        /*27 default gap3 value*/
	floppy_command(floppy_params->data_length);       /*default value for data length*/

	//kprintf("FDD: BPS: %u, SPT: %u, GL: %u, DL: %u\n", floppy_params->bytes_per_sector, floppy_params->sectors_per_track, floppy_params->gap_length, floppy_params->data_length);

	wait_irq(FLOPPY_IRQ);
	//kprintf("We got values ");
	for (a = 0; a < 7; a++) { /* TODO: Put these values somewhere? */
		floppy_wait_data();
		inportb(FLOPPY_FIRST + DATA_FIFO);
		//kprintf("%d ", inportb(FLOPPY_FIRST + DATA_FIFO));
	}
	//kprintf(" from floppy controller after reading\n");
	floppy_prepare_motor_off(drive);
	return 0;
}

int floppy_read_one_block(struct floppy *self, uint64_t num64, void * buf)
{
	int sector;
	int head;
	int cylinder;
	int retval;
	size_t num = num64;

	head = (num / floppy_params->sectors_per_track) & 1;
	cylinder = num / (floppy_params->sectors_per_track * 2);
	sector = (num % (floppy_params->sectors_per_track)) + 1;

	retval = floppy_rw_sector(self->num, sector, head, cylinder, (uintptr_t) floppy_buffer, 0);
	if (retval == 0) {
		memcpy(buf, floppy_buffer, self->blockdev.block_size);
	}
	return retval;
}

int floppy_write_one_block(struct floppy *self, uint64_t num64, const void * buf)
{
	int sector;
	int head;
	int cylinder;
	size_t num = num64;
	kprintf("Write at %d\n", num);

	head = (num / floppy_params->sectors_per_track) & 1;
	cylinder = num / (floppy_params->sectors_per_track * 2);
	sector = (num % (floppy_params->sectors_per_track)) + 1;

	memcpy(floppy_buffer, buf, self->blockdev.block_size);
	return floppy_rw_sector(self->num, sector, head, cylinder, (uintptr_t) floppy_buffer, 1);
}

