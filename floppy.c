#include <floppy.h>
#include <mem.h>
#include <io.h>
#include <irq.h>
#include <screen.h>
#include <bit.h>
#include <timer.h>

floppy_parameters floppy_params;
int cylinder, status_0;

const char * fd_types[6] = { "no floppy drive", "360KB 5.25\" floppy drive", "1.2MB 5.25\" floppy drive", "720KB 3.5\" floppy drive", "1.44MB 3.5\" floppy drive", "2.88MB 3.5\" floppy drive"};

void install_floppy() {
	extern void irq6();
	char detect_floppy;
	
	memcpy(&floppy_params, (void*)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));

	outportb(0x70, 0x10);
	detect_floppy = inportb(0x71);

	print("We found ");
	print(fd_types[detect_floppy >> 4]);
	print(" at fd0\n");

	print("We found ");
	print(fd_types[detect_floppy & 0xF]);
	print(" at fd1\n");

}

void reset_floppy() {
	outportb((FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER), 0x00); /*disable controller*/
	outportb(FLOPPY_FIRST + CONFIGURATION_CONTROL_REGISTER, 0);
	outportb(FLOPPY_FIRST + DATA_RATE_SELECT_REGISTER, 0);
	outportb((FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER), 0x0c); /*enable controller*/

	wait_irq(6);
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
	send_command(floppy_params.headload_ndma & 254);
}	

void send_command(char command) {
	wait_floppy();
	outportb(FLOPPY_FIRST + DATA_FIFO, command);
}

void wait_floppy() {
	while(!((inportb(FLOPPY_FIRST + MAIN_STATUS_REGISTER)) & 0x80))
		;
}

void sense_interrupt() {
	print("going to sense interrupt\n");
	send_command(SENSE_INTERRUPT);
	wait_floppy_data();
	status_0 = inportb(FLOPPY_FIRST + DATA_FIFO);
	wait_floppy_data();
	cylinder = inportb(FLOPPY_FIRST + DATA_FIFO);
	print("done\n");
}

void calibrate_drive(){
	outportb(FLOPPY_FIRST + DIGITAL_OUTPUT_REGISTER, 0x1C); /* motor on*/
	wait(500);	/* wait motor */
	send_command(RECALIBRATE); /* (re)calibrate drive*/
	send_command(0); /* drive 0 */
	wait_irq(6);  /*wait for interrupt from controller*/
	sense_interrupt();
	outportb(FLOPPY_FIRST, 0); /* motor off */
}
