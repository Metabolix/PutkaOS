#ifndef _FLOPPY_H
#define _FLOPPY_H

#include <stddef.h>
#include <timer.h>
#include <blockdev.h>


#define FLOPPY_FIRST    0x03F0
#define FLOPPY_SECOND   0x0370

#define STATUS_REGISTER_A         0x0 /* read-only */
#define STATUS_REGISTER_B         0x1 /* read-only */
#define DIGITAL_OUTPUT_REGISTER   0x2
#define TAPE_DRIVE_REGISTER       0x3
#define MAIN_STATUS_REGISTER      0x4 /* read-only */ 
#define DATA_RATE_SELECT_REGISTER 0x4 /* write-only */ 
#define DATA_FIFO                 0x5
#define DIGITAL_INPUT_REGISTER    0x7 /* read-only */
#define CONFIGURATION_CONTROL_REGISTER  0x7 /* write only */


#define READ_TRACK 0x2
#define SPECIFY  0x3
#define SENSE_DRIVE_STATUS 0x4
#define WRITE_DATA 0x5
#define READ_DATA 0x46
#define RECALIBRATE 0x7
#define SENSE_INTERRUPT 0x8
#define SEEK 0xf

#define MAX_DRIVES 2

typedef struct {
	unsigned char motor;
	unsigned char type;
	unsigned char track;
	unsigned char status;
	struct timer_job djob;
} fd;


typedef struct {
	unsigned char steprate_headunload;
	unsigned char headload_ndma;
	unsigned char motor_delay_off; /*specified in clock ticks*/
	unsigned char bytes_per_sector;
	unsigned char sectors_per_track;
	unsigned char gap_length;
	unsigned char data_length; /*used only when bytes per sector == 0*/
	unsigned char format_gap_length;
	unsigned char filler;
	unsigned char head_settle_time; /*specified in milliseconds*/
	unsigned char motor_start_time; /*specified in 1/8 seconds*/
}__attribute__ ((packed)) floppy_parameters;

#define DISK_PARAMETER_ADDRESS 0x000fefc7

void install_floppy();
void reset_floppy();
void wait_floppy_data();
void configure_drive();
void send_command(char command);
void wait_floppy();
void sense_interrupt();
void calibrate_drive(char drive);
int seek_track(char drive, int track);
void reset_flipflop_dma();
void init_dma_floppy(unsigned long buffer, size_t len, int write);
int read_sector(char drive, unsigned char sector, unsigned char head, unsigned char cylinder, unsigned long buffer);
int write_sector(char drive, unsigned char sector, unsigned char head, unsigned char cylinder, unsigned long buffer);
int read_block(BLOCK_DEVICE *self, size_t num, void * buf);
int write_block(BLOCK_DEVICE *self, size_t num, const void * buf);
void motor_0_off();
void motor_1_off();

#endif
