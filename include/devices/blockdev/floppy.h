#ifndef _FLOPPY_H
#define _FLOPPY_H 1

#include <stddef.h>
#include <stdint.h>
#include <timer.h>
#include <devices/blockdev/blockdev.h>


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
#define WRITE_DATA 0x45
#define READ_DATA 0x46
#define RECALIBRATE 0x7
#define SENSE_INTERRUPT 0x8
#define SEEK 0xf

#define MAX_DRIVES 2
#define FLOPPY_IRQ 6
struct floppy {
	BD_DEVICE blockdev;
	uint_t num;
	uint8_t motor, type, track, status;
	timer_id_t motor_off_timer;
};

struct floppy_parameters {
	uint8_t steprate_headunload;
	uint8_t headload_ndma;
	uint8_t motor_delay_off; /*specified in clock ticks*/
	uint8_t bytes_per_sector;
	uint8_t sectors_per_track;
	uint8_t gap_length;
	uint8_t data_length; /*used only when bytes per sector == 0*/
	uint8_t format_gap_length;
	uint8_t filler;
	uint8_t head_settle_time; /*specified in milliseconds*/
	uint8_t motor_start_time; /*specified in 1/8 seconds*/
}__attribute__ ((packed));

#define FLOPPY_PARAMETER_ADDRESS ((void*)0x000fefc7)

extern void floppy_init(void);
extern void floppy_cp_mem(void);
extern void floppy_reset(void);

#endif
