#include <storage/floppy.h>
#include <storage/blockdev.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <screen.h>
#include <bit.h>
#include <timer.h>
#include <panic.h>

floppy_parameters floppy_params;
int cylinder, status_0;
#define wait_irq(a) kwait(0, 20*1000)

BD_DEVICE fd_devices[2] = {
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

		(read_one_block_t) read_one_block,
		(write_one_block_t) write_one_block,
		(read_blocks_t) 0,
		(write_blocks_t) 0
	},
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

		(read_one_block_t) read_one_block,
		(write_one_block_t) write_one_block,
		(read_blocks_t) 0,
		(write_blocks_t) 0
	}
};

const char * fd_types[6] = {
	"no floppy drive",
	"360KB 5.25\" floppy drive",
	"1.2MB 5.25\" floppy drive",
	"720KB 3.5\" floppy drive",
	"1.44MB 3.5\" floppy drive",
	"2.88MB 3.5\" floppy drive"
};

struct fd fds[2];
char fdbuf[512];
unsigned int fd_seeked = 0;
unsigned int motors = 0;

void motor_0_off(void) {motor_off(0);}
void motor_1_off(void) {motor_off(1);}
void (*motor_stop[2])() = {motor_0_off, motor_1_off};

void floppy_handler(void) { }

void install_floppy(void)
{
	unsigned char detect_floppy;

	memcpy(&floppy_params, (void*)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));
	memset(fds, 0, sizeof(struct fd) * 2);
	memset(fdbuf, 0, 512);

	if (floppy_params.bytes_per_sector > 2) {
		print("FDD: ERROR: Sector size bigger than 512 bytes (disabled floppies)\n");
		fds[0].type = 0;
		fds[1].type = 0;
		return;
	}

	outportb(0x70, 0x10);
	detect_floppy = inportb(0x71);

	fds[0].type = detect_floppy >> 4;
	fds[1].type = detect_floppy & 0xF;

	kprintf("FDD: We found %s at fd0\n", fd_types[detect_floppy >> 4]);
	if (fds[0].type != 4 && fds[0].type) {
		print("FDD: We don't support it\n");
	}
	kprintf("FDD: We found %s at fd1\n", fd_types[detect_floppy & 0xF]);
	if (fds[1].type != 4 && fds[1].type) {
		print("FDD: We don't support it\n");
	}

	if (fds[0].type == 4) {
		device_insert((DEVICE*)(fd_devices + 0));

		/* with 1.44MB 3.5" floppy disk, 2 sides and 80 tracks per side */
		fd_devices[0].block_size = 128 * (1 << floppy_params.bytes_per_sector);
		fd_devices[0].block_count = 2 * 80 * floppy_params.sectors_per_track;
	} else {
		fd_devices[0].std.dev_type = DEV_TYPE_NONE;
	}

	if (fds[1].type == 4) {
		device_insert((DEVICE*)(fd_devices + 1));

		/* with 1.44MB 3.5" floppy disk, 2 sides and 80 tracks per side */
		fd_devices[1].block_size = 128 * (1 << floppy_params.bytes_per_sector);
		fd_devices[1].block_count = 2 * 80 * floppy_params.sectors_per_track;
	} else {
		fd_devices[1].std.dev_type = DEV_TYPE_NONE;
	}

	install_irq_handler(6, (void*)floppy_handler);
}

void reset_floppy(void)
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
		sense_interrupt();
	}

	outportb(FLOPPY_FIRST + CONFIGURATION_CONTROL_REGISTER, 0);

	configure_drive();

	if (fds[0].type) {
		calibrate_drive(0);
	}
	if (fds[1].type) {
		calibrate_drive(1);
	}
	print("FDD: Calibrated drives\n");
}

void wait_floppy_data(void)
{
	/* While RQM and DIO aren't 1 */
	while ((inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0xd0) != 0xd0);
}


void configure_drive(void)
{
	send_command(SPECIFY);
	send_command(floppy_params.steprate_headunload);
	send_command(floppy_params.headload_ndma & 254);
	print("FDD: Drive configured\n");
}

void send_command(unsigned char command)
{
	wait_floppy();
	outportb(FLOPPY_FIRST + DATA_FIFO, command);
}

void wait_floppy(void)
{
	while (!(inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x80));
}

void sense_interrupt(void)
{
	send_command(SENSE_INTERRUPT);
	wait_floppy_data();
	fds[fd_seeked].status = inportb(FLOPPY_FIRST + DATA_FIFO);
	wait_floppy_data();
	fds[fd_seeked].track = inportb(FLOPPY_FIRST + DATA_FIFO);
}

void calibrate_drive(unsigned int drive)
{
	if (drive >= MAX_DRIVES || !fds[drive].type) {
		return;
	}

	prepare_wait_irq(FLOPPY_IRQ);
	send_command(RECALIBRATE); /* (re)calibrate drive*/
	send_command(drive);
	wait_irq(FLOPPY_IRQ);
	kprintf("FDD: fd%u calibrated\n", drive);
}

void init_dma_floppy(unsigned long buffer, size_t len, int write) {
	asm("cli");
	outportb(0x0a, 0x06);      /* mask DMA channel 2 */
	reset_flipflop_dma();
	outportb(0x4, buffer & 0xFF);
	outportb(0x4, (buffer >> 8) & 0xFF);
	reset_flipflop_dma();
	len--;
	outportb(0x5, len & 0xFF);
	outportb(0x5, len >> 8);
	outportb(0x81, buffer >> 16);
	outportb(0x0b, (write ? 0x48: 0x44) + 2);  /* single transfer, write or read, channel 2 */
	outportb(0x0a, 0x02);          /* unmask DMA channel 2 */
	asm("sti");
}

void motor_on(unsigned int drive) {
	if (drive >= MAX_DRIVES) {
		return;
	}
	if (fds[drive].motor) { /* motor already on */
		return;
	}
	if (fds[drive].motor_off_timer) {
		ktimer_stop(fds[drive].motor_off_timer);
		fds[drive].motor_off_timer = 0;
	}
	fds[drive].motor = 1;
	motors |= (1 << drive);
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC + (motors << 4)); /* motor on */
	kwait(0, 1000 * 500); /* wait it spins up */
}

void prepare_motor_off(unsigned int drive) {
	if (fds[drive].motor_off_timer) {
		ktimer_stop(fds[drive].motor_off_timer);
		fds[drive].motor_off_timer = 0;
	}
	fds[drive].motor_off_timer = ktimer_start(motor_stop[drive], 5000, 1);
}

int seek_track(unsigned int drive, unsigned int track)
{
	if (drive >= MAX_DRIVES){
		return -1;
	}

	if (track == fds[drive].track) {/* already on right cylinder */
		/*kprintf("FDD: fd%u already on right cylinder\n", drive);*/
		return 0;
	}

	motor_on(drive);

	prepare_wait_irq(FLOPPY_IRQ);
	send_command(SEEK);
	send_command(drive);
	send_command(track);
	wait_irq(FLOPPY_IRQ);

	fd_seeked = drive;

	sense_interrupt();
	if (track != fds[drive].track) {
		kprintf("FDD: ERROR: fd%u couldn't seek\n", drive);
		prepare_motor_off(drive);
		return -2;
	}
	/*kprintf("FDD: fd%u: Seeked track succesfully\n", drive);*/
	prepare_motor_off(drive);
	return 0;
}

void reset_flipflop_dma(void) {
	outportb(0xC, 0);
}

void motor_off(unsigned int drive)
{
	if (fds[drive].motor == 0) {
		return;
	}
	motors &= ~(1 << drive);
	fds[drive].motor = 0;
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC | motors << 4);
	kprintf("FDD: Motor %u off\n", drive);
}

int rw_sector(unsigned int drive, unsigned char sector, unsigned char head, unsigned char cylinder, unsigned long buffer, int write)
{
	int a;
alku:
	if (drive >= MAX_DRIVES || !fds[drive].type) {
		return -1;
	}

	if (inportb(FLOPPY_FIRST + DIGITAL_INPUT_REGISTER) & 0x80) {
		/* disk was changed */
		seek_track(drive, 1);
		calibrate_drive(drive);
		motor_off(drive);
		if (inportb(FLOPPY_FIRST + DIGITAL_INPUT_REGISTER) & 0x80) {
			kprintf("FDD: No floppy in fd%u\n", drive);
			return -2;
		} else {
			print("FDD: Floppy changed, trying again...\n");
			goto alku;
		}
	}

	if (seek_track(drive, cylinder))
	if (seek_track(drive, cylinder))
	if (seek_track(drive, cylinder)) {
		return -1; /* three seeks? */
	}

	/* seek_track actually starts motor already, but... */
	motor_on(drive);

	if (!inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x20) {
		panic("Non-dma floppy transfer?\n");
	}

	/* block size */
	init_dma_floppy(buffer, 512, 0);

	kwait(0, 1000 * floppy_params.head_settle_time);
	wait_floppy();

	prepare_wait_irq(FLOPPY_IRQ);
	send_command(write ? WRITE_DATA : READ_DATA);
	send_command((head << 2) | drive);
	send_command(cylinder);
	send_command(head);
	send_command(sector);
	send_command(floppy_params.bytes_per_sector);  /*sector size = 128*2^size*/
	send_command(floppy_params.sectors_per_track); /*last sector*/
	send_command(floppy_params.gap_length);        /*27 default gap3 value*/
	send_command(floppy_params.data_length);       /*default value for data length*/

	//kprintf("FDD: BPS: %u, SPT: %u, GL: %u, DL: %u\n", floppy_params.bytes_per_sector, floppy_params.sectors_per_track, floppy_params.gap_length, floppy_params.data_length);

	wait_irq(FLOPPY_IRQ);
	//kprintf("We got values ");
	for (a = 0; a < 7; a++) { /* TODO: Put these values somewhere? */
		wait_floppy_data();
		inportb(FLOPPY_FIRST + DATA_FIFO);
		//kprintf("%d ", inportb(FLOPPY_FIRST + DATA_FIFO));
	}
	//kprintf(" from floppy controller after reading\n");
	prepare_motor_off(drive);
	return 0;
}

int read_one_block(BD_DEVICE *self, uint64_t num64, void * buf)
{
	int sector;
	int head;
	int cylinder;
	int retval;
	size_t num = num64;

	head = (num / floppy_params.sectors_per_track) & 1;
	cylinder = num / (floppy_params.sectors_per_track * 2);
	sector = (num % (floppy_params.sectors_per_track)) + 1;

	retval = rw_sector(self->std.name[2] - '0', sector, head, cylinder,(unsigned long) fdbuf, 0);
	if (retval == 0) {
		memcpy(buf, fdbuf, self->block_size);
	}
	return retval;
}

int write_one_block(BD_DEVICE *self, uint64_t num64, const void * buf)
{
	int sector;
	int head;
	int cylinder;
	size_t num = num64;

	head = (num / floppy_params.sectors_per_track) & 1;
	cylinder = num / (floppy_params.sectors_per_track * 2);
	sector = (num % (floppy_params.sectors_per_track)) + 1;

	memcpy(fdbuf, buf, self->block_size);
	return rw_sector(self->std.name[2] - '0', sector, head, cylinder, (unsigned long)fdbuf, 1);
}
