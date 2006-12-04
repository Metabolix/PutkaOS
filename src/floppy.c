#include <floppy.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <screen.h>
#include <bit.h>
#include <timer.h>
#include <panic.h>
#include <blockdev.h>

floppy_parameters floppy_params;
int cylinder, status_0;

BLOCK_DEVICE fd_devices[2];

const char * fd_types[6] = { "no floppy drive", "360KB 5.25\" floppy drive", "1.2MB 5.25\" floppy drive", "720KB 3.5\" floppy drive", "1.44MB 3.5\" floppy drive", "2.88MB 3.5\" floppy drive"};

fd fds[2];
int fd_seeked = 0;
char motors = 0;


void *motor_stop[2] = {&motor_0_off, &motor_1_off};

void floppy_handler() { }

void install_floppy() {
	char detect_floppy;

	memcpy(&floppy_params, (void*)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));
	memset(&fds, 0, sizeof(fd) * 2);

	outportb(0x70, 0x10);
	detect_floppy = inportb(0x71);

	fds[0].type = detect_floppy >> 4;
	fds[1].type = detect_floppy & 0xF;

	if(fds[0].type == 4) {
		fd_devices[0].dev_type = DEV_TYPE_FLOPPY;
		fd_devices[0].name = "fd0";
		fd_devices[0].index = 0;
		fd_devices[0].block_size = 128 * 2 << floppy_params.bytes_per_sector;
		fd_devices[0].block_count = 2 * 80 * floppy_params.sectors_per_track; /* with 1.44MB 3.5" floppy disk, 2 sides and 80 tracks per side */
		fd_devices[0].read_block = read_block;
		fd_devices[0].write_block = write_block;
	} else {
		fd_devices[0].dev_type = DEV_TYPE_NONE;
	}

	if(fds[1].type == 4) {
		fd_devices[1].dev_type = DEV_TYPE_FLOPPY;
		fd_devices[1].name = "fd1";
		fd_devices[1].index = 1;
		fd_devices[1].block_size = 128 * 2 << floppy_params.bytes_per_sector;
		fd_devices[1].block_count = 2 * 80 * floppy_params.sectors_per_track; /* with 1.44MB 3.5" floppy disk, 2 sides and 80 tracks per side */
		fd_devices[1].read_block = read_block;
		fd_devices[1].write_block = write_block;
	} else {
		fd_devices[1].dev_type = DEV_TYPE_NONE;
	}



	kprintf("FDD: We found %s at fd0\n", fd_types[detect_floppy >> 4]);
	if(fds[0].type != 4 && fds[0].type)
		print("FDD: We don't support it\n");
	kprintf("FDD: We found %s at fd1\n", fd_types[detect_floppy & 0xF]);
	if(fds[1].type != 4 && fds[1].type)
		print("FDD: We don't support it\n");

	install_irq_handler(6, (void*)floppy_handler);
}

void reset_floppy() {
	int a;

	prepare_wait_irq(6);
	outportb((FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER), 0x00); /*disable controller*/
	kwait(50);
	outportb((FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER), 0x0c); /*enable controller*/

	outportb(FLOPPY_FIRST + CONFIGURATION_CONTROL_REGISTER, 0);
	outportb(FLOPPY_FIRST + DATA_RATE_SELECT_REGISTER, 0);

	wait_irq(6);
	for(a = 0; a < 4; a++)
		sense_interrupt();
	configure_drive();
	if(fds[0].type)
		calibrate_drive(0);
	if(fds[1].type)
		calibrate_drive(1);
}

void wait_floppy_data() {
	while((inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0xd0) != 0xd0) /* While RQM, DIO and CMD BUSY aren't 1 */
		;
}
	
	
void configure_drive(){
	send_command(SPECIFY);
	send_command(floppy_params.steprate_headunload);
	send_command(floppy_params.headload_ndma << 1);
	print("FDD: Drive configured\n");
}	

void send_command(char command) {
	wait_floppy();
	outportb(FLOPPY_FIRST + DATA_FIFO, command);
}

void wait_floppy() {
	while(!(inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x80));
}

void sense_interrupt() {
	send_command(SENSE_INTERRUPT);
	wait_floppy_data();
	fds[fd_seeked].status = inportb(FLOPPY_FIRST + DATA_FIFO);
	wait_floppy_data();
	fds[fd_seeked].track = inportb(FLOPPY_FIRST + DATA_FIFO);
}

void calibrate_drive(char drive) {
	if(drive >= 0 && drive < MAX_DRIVES) {
		if(!fds[(int)drive].type) {
			return;
		}

		prepare_wait_irq(6);
		send_command(RECALIBRATE); /* (re)calibrate drive*/
		send_command(drive);
		wait_irq(6);  /*wait for interrupt from controller*/
		sense_interrupt();
		kprintf("FDD: fd%u calibrated\n", drive);
	}
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

void motor_on(char drive) {
	if(drive < 0 || drive >= MAX_DRIVES) {
		return;
	}
	if(fds[(int)drive].motor) { /* motor already on */
		return;
	}
	fds[(int)drive].djob.function = motor_stop[(int)drive];
	kunregister_job(&fds[(int)drive].djob);
	fds[(int)drive].motor = 1;
	motors |= (1 << drive);
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC + (motors << 4)); /* motor on */
	kwait(500);
}

void prepare_motor_off(char drive) {
	fds[(int)drive].djob.times = 1;
	fds[(int)drive].djob.function = motor_stop[(int)drive];
	fds[(int)drive].djob.time = kget_ticks() + 500;
	kregister_job(&(fds[(int)drive].djob));
}

int seek_track(char drive, int track) {
	if(drive < 0 || drive >= MAX_DRIVES){
		return -1;
	}

	if(track == fds[(int)drive].track) /* already on right cylinder */
		return 0;

	motor_on(drive);

	prepare_wait_irq(6);
	send_command(SEEK);
	send_command(drive);
	send_command(track);

	wait_irq(6);
	fd_seeked = drive;

	sense_interrupt();
	if(track != cylinder) {
		print("FDD: ERROR: fd0 couldn't seek\n");
		return -2;
	}
	print("FDD: Seeked track succesfully\n");
	prepare_motor_off(drive);
	return 0;
}

void reset_flipflop_dma() {
        outportb(0xC, 0);
}

void motor_0_off() {
	motors &= ~1;
	fds[0].motor = 0;
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC | motors << 4);
	print("FDD: Motor 0 off\n");
}

void motor_1_off() {
	motors &= ~2;
	fds[1].motor = 0;
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0xC | motors << 4);
	print("FDD: Motor 1 off\n");
}

int read_sector(char drive, unsigned char sector, unsigned char head, unsigned char cylinder, unsigned long buffer)
{
	if(drive < 0 || drive >= MAX_DRIVES)
		return -1;

	if(!fds[(int)drive].type) {
		return -1;
	}
	motor_on(drive);
	if(seek_track(drive, cylinder)) {
		return -1; /* uhm, should we try to seek few times more? */
	}

	if(!inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x20)
		panic("Non-dma floppy transfer?\n");

	init_dma_floppy(buffer, 512, 0); /* block size */

	kwait(floppy_params.head_settle_time);
	prepare_wait_irq(6);
	send_command(READ_DATA);
	send_command((head << 2) & drive);
	send_command(cylinder);
	send_command(head);
	send_command(sector);
	send_command(floppy_params.bytes_per_sector);  /*sector size = 128*2^size*/
	send_command(floppy_params.sectors_per_track); /*last sector*/
	send_command(floppy_params.gap_length);        /*27 default gap3 value*/
	send_command(floppy_params.data_length);       /*default value for data length*/

	kprintf("FDD: BPS: %u, SPT: %u, GL: %u, DL: %u\n", floppy_params.bytes_per_sector, floppy_params.sectors_per_track, floppy_params.gap_length, floppy_params.data_length);

	wait_irq(6);

	prepare_motor_off(drive);
	return 0;
}

int write_sector(char drive, unsigned char sector, unsigned char head, unsigned char cylinder, unsigned long buffer)
{
	if(drive < 0 || drive >= MAX_DRIVES)
		return -1;

	if(!fds[(int)drive].type) {
		return -1;
	}
	motor_on(drive);
	if(seek_track(drive, cylinder)) {
		return -1; /* uhm, should we try to seek few times more? */
	}

	if(!inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x20)
		panic("Non-dma floppy transfer?\n");

	init_dma_floppy(buffer, 512, 1); /* block size */

	kwait(floppy_params.head_settle_time);
	prepare_wait_irq(6);
	send_command(WRITE_DATA);
	send_command((head << 2) & drive);
	send_command(cylinder);
	send_command(head);
	send_command(sector);
	send_command(floppy_params.bytes_per_sector);  /*sector size = 128*2^size*/
	send_command(floppy_params.sectors_per_track); /*last sector*/
	send_command(floppy_params.gap_length);        /*27 default gap3 value*/
	send_command(floppy_params.data_length);       /*default value for data length*/

	kprintf("FDD: BPS: %u, SPT: %u, GL: %u, DL: %u\n", floppy_params.bytes_per_sector, floppy_params.sectors_per_track, floppy_params.gap_length, floppy_params.data_length);

	wait_irq(6);

	prepare_motor_off(drive);
	return 0;
}


int read_block(BLOCK_DEVICE *self, size_t num, void * buf) {
	int sector;
	int head;
	int cylinder;

	head = (num % 2);
	cylinder = num / floppy_params.sectors_per_track;
	sector = num % floppy_params.sectors_per_track + 1;

	return read_sector(self->index, sector, head, cylinder,(unsigned long) buf);
}

int write_block(BLOCK_DEVICE *self, size_t num, const void * buf) {
	int sector;
	int head;
	int cylinder;

	head = (num % 2);
	cylinder = num / floppy_params.sectors_per_track;
	sector = num % floppy_params.sectors_per_track + 1;

	return read_sector(self->index, sector, head, cylinder,(unsigned long) buf);
}

