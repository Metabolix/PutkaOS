#include <floppy.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <screen.h>
#include <bit.h>
#include <timer.h>
#include <panic.h>

floppy_parameters floppy_params;
int cylinder, status_0;

const char * fd_types[6] = { "no floppy drive", "360KB 5.25\" floppy drive", "1.2MB 5.25\" floppy drive", "720KB 3.5\" floppy drive", "1.44MB 3.5\" floppy drive", "2.88MB 3.5\" floppy drive"};

void install_floppy() {
	char detect_floppy;
	
	memcpy(&floppy_params, (void*)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));

	outportb(0x70, 0x10);
	detect_floppy = inportb(0x71);

	kprintf("We found %s at fd0", fd_types[detect_floppy >> 4]);
	kprintf("We found %s at fd1", fd_types[detect_floppy & 0xF]);

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
	calibrate_drive();
}

void wait_floppy_data() {
	while((inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0xd0) != 0xd0) /* While RQM, DIO and CMD BUSY aren't 1 */
		;
}
	
	
void configure_drive(){
	send_command(SPECIFY);
	send_command(floppy_params.steprate_headunload);
	send_command(floppy_params.headload_ndma << 1);
	print("Drive configured\n");
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
	status_0 = inportb(FLOPPY_FIRST + DATA_FIFO);
	wait_floppy_data();
	cylinder = inportb(FLOPPY_FIRST + DATA_FIFO);
}

void calibrate_drive(){
	prepare_wait_irq(6);
	send_command(RECALIBRATE); /* (re)calibrate drive*/
	send_command(0); /* drive 0 */
	wait_irq(6);  /*wait for interrupt from controller*/
	sense_interrupt();
	print("Drive calibrated\n");
}

void init_dma_floppy(unsigned long buffer, int len) {
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
	outportb(0x0b, 0x46);  /* single transfer, read, channel 2 */
        outportb(0x0a, 0x02);          /* unmask DMA channel 2 */
	asm("sti");
}

int seek_track(int track) {
	print("Seek_track\n");
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0x1C); /* motor on*/
	kwait(500);
        
	prepare_wait_irq(6);
	send_command(SEEK);
        send_command(0);
        send_command(track);

	wait_irq(6);
	sense_interrupt();
	if(track != cylinder)
		print("ERROR: fd0 couldn't seek\n");
	print("Seeked track succesfully\n");
        kwait(15);
	return 0;
}

void reset_flipflop_dma() {
        outportb(0xC, 0);
}

void read_sector(unsigned char sector, unsigned char head, unsigned char cylinder, unsigned long buffer)
{
        seek_track(sector);
	if(!inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER) & 0x20)
		panic("Non-dma o_O?\n");

        init_dma_floppy(buffer, 512);
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0x1C); /* motor on*/
	kwait(500);

        print("prepared dma read\n");
        kwait(floppy_params.head_settle_time);
        prepare_wait_irq(6);
	send_command(READ_DATA);
        send_command(head<<2);
        send_command(cylinder);
        send_command(head);
        send_command(sector);
        send_command(floppy_params.bytes_per_sector);  /*sector size = 128*2^size*/
        send_command(floppy_params.sectors_per_track); /*last sector*/
        send_command(floppy_params.gap_length);        /*27 default gap3 value*/
        send_command(floppy_params.data_length);       /*default value for data length*/

        wait_irq(6);
}

