#include <devices/ports/serial.h>
#include <stdint.h>
#include <kprintf.h>
#include <stdint.h>
#include <io.h>
#include <list.h>

// http://www.beyondlogic.org/serial/serial.htm#12

#define SERIAL_MAX_BAUDS ((uint32_t) 115200)

#define SERIAL_THB (0) /* W  Transmitter Holding Buffer */
#define SERIAL_RCB (0) /* R  Receiver Buffer */
#define SERIAL_IER (1) /* RW Interrupt Enable Register */
#define SERIAL_IIR (2) /* R  Interrupt Identification Register */
#define SERIAL_FCR (2) /* W  FIFO Control Register */
#define SERIAL_LCR (3) /* RW Line Control Register */
#define SERIAL_MCR (4) /* RW Modem Control Register */
#define SERIAL_LSR (5) /* R  Line Status Register */
#define SERIAL_MSR (6) /* R  Modem Status Register */
#define SERIAL_SXX (7) /* RW Scratch Register */

/* Divisor latch */
#define SERIAL_DL_LOW (0)
#define SERIAL_DL_HIGH (1)

/* RW Interrupt Enable Register */
#define SERIAL_IER_LOWPOWER ((uint8_t)0x20)
#define SERIAL_IER_SLEEP ((uint8_t)0x10)
#define SERIAL_IER_MODEM_STATUS_INT ((uint8_t)0x08)
#define SERIAL_IER_REC_LINE_STATUS_INT ((uint8_t)0x04)
#define SERIAL_IER_TRANS_REG_EMPTY_INT ((uint8_t)0x02)
#define SERIAL_IER_DATA_AVAILABLE_INT ((uint8_t)0x01)

/* R  Interrupt Identification Register */
#define SERIAL_IIR_FIFO ((uint8_t)0xc0)
#define SERIAL_IIR_16750_FIFO_64 ((uint8_t)0x20)
#define SERIAL_IIR_16550_TIMEOUT_INT ((uint8_t)0x08)
#define SERIAL_IIR_WHAT_INT_PENDING ((uint8_t)0x06)
#define SERIAL_IIR_INT_PENDING ((uint8_t)0x01)

/* W  FIFO Control Register */
#define SERIAL_FCR_INT_BUF_LEVEL ((uint8_t)0xc0)
#define SERIAL_FCR_16750_FIFO_64 ((uint8_t)0x20)
#define SERIAL_FCR_DMA ((uint8_t)0x08)
#define SERIAL_FCR_CLR_TRANSMIT ((uint8_t)0x04)
#define SERIAL_FCR_CLR_RECEIVE ((uint8_t)0x02)
#define SERIAL_FCR_ENABLE_FIFO ((uint8_t)0x01)

/* RW Line Control Register */
#define SERIAL_LCR_DLAB ((uint8_t)0x80)
#define SERIAL_LCR_BREAK ((uint8_t)0x40)
#define SERIAL_LCR_PARITY ((uint8_t)0x38)
#define SERIAL_LCR_STOP_BIT ((uint8_t)0x04)
#define SERIAL_LCR_WORD_LEN ((uint8_t)0x03)

/* RW Modem Control Register */
#define SERIAL_MCR_ ((uint8_t)0x80)
#if 0
  Bit 7  Reserved
  Bit 6  Reserved
  Bit 5  Autoflow Control Enabled (16750 only)
  Bit 4  LoopBack Mode 
  Bit 3  Aux Output 2
  Bit 2  Aux Output 1
  Bit 1  Force Request to Send
  Bit 0  Force Data Terminal Ready
#endif

/* R  Line Status Register */
#define SERIAL_LSR_ ((uint8_t)0x80)
#if 0
  Bit 7  Error in Received FIFO
  Bit 6  Empty Data Holding Registers 
  Bit 5  Empty Transmitter Holding Register
  Bit 4  Break Interrupt 
  Bit 3  Framing Error
  Bit 2  Parity Error
  Bit 1  Overrun Error
  Bit 0  Data Ready
#endif

/* R  Modem Status Register */
#define SERIAL_MSR_ ((uint8_t)0x80)
#if 0
  Bit 7  Carrier Detect
  Bit 6  Ring Indicator 
  Bit 5  Data Set Ready
  Bit 4  Clear To Send 
  Bit 3  Delta Data Carrier Detect
  Bit 2  Trailing Edge Ring Indicator
  Bit 1  Delta Data Set Ready
  Bit 0  Delta Clear to Send
#endif

struct serial_port {
	uint16_t addr;
};

LIST_TYPE(serial_port, struct serial_port);

list_of_serial_port serial_ports;

static void set_divisor(uint16_t base, uint16_t divisor)
{
	outportb(base + SERIAL_LCR, inportb(base + SERIAL_LCR) | SERIAL_LCR_DLAB);
	outportb(base + SERIAL_DL_LOW, (uint8_t)(divisor & 0xff));
	outportb(base + SERIAL_DL_HIGH, (uint8_t)((divisor >> 8) & 0xff));
	outportb(base + SERIAL_LCR, inportb(base + SERIAL_LCR) & ~SERIAL_LCR_DLAB);
}

static uint32_t set_bauds(uint16_t base, uint32_t bauds)
{
	uint16_t divisor;
	uint32_t real_bauds, miss;
	uint32_t best_bauds, least_miss;
	uint32_t i2, i3, i5;
	int c2, c3, c5;
	
	if (bauds >= SERIAL_MAX_BAUDS) {
		set_divisor(base, 1);
		return SERIAL_MAX_BAUDS;
	}
	if (bauds <= 2) {
		set_divisor(base, SERIAL_MAX_BAUDS / 2);
		return 2;
	}
	
	best_bauds = SERIAL_MAX_BAUDS;
	least_miss = SERIAL_MAX_BAUDS;
	
	for (c2 = 0, i2 = 1; c2 <= 9; ++c2, i2 *= 2) {
		for (c3 = 0, i3 = 1; c3 <= 2; ++c3, i3 *= 3) {
			for (c5 = 0, i5 = 1; c5 <= 2; ++c5, i5 *= 5) {
				real_bauds = i2 * i3 * i5;
				miss = ((bauds > real_bauds) ? (bauds - real_bauds) : (real_bauds - bauds));
				if (miss < least_miss) {
					least_miss = miss;
					best_bauds = real_bauds;
					if (miss == 0) {
						goto ulkopuoli;
					}
				}
			}
		}
	}
ulkopuoli:
	divisor = SERIAL_MAX_BAUDS / best_bauds;
	set_divisor(base, divisor);
	return best_bauds;
}

int serial_init(void)
{
	int i;
	struct serial_port port;
	uint16_t *portaddr = (void*) 0x400;
	
	list_init(serial_ports);
	
	for (i = 0; i < 4; ++i) {
		if (!portaddr[i]) continue;
		port.addr = portaddr[i];
		set_bauds(port.addr, 38400);
		//list_insert(serial_ports, port);
	}
	return 0;
}

int serial_uninit(void)
{
	//list_uninit(serial_ports);
	return 0;
}
