/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * msp430-i2c.c -- MSP430 i2c interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <msp430.h>
#include "types.h"
#include "assert.h"
#include "msp430-defs.h"
#include "msp430-gpio.h"
#include "i2c.h"

#define CONFIG_PLAT_RUDDOCK2	1

#if CONFIG_PLAT_RUDDOCK2
#define USCI_UNIT	B
#define USCI_CHAN	1
// Pin definitions for this unit
#define	I2C_SCL		GPIO(5,4)
#define I2C_SDA		GPIO(3,7)

#else

#endif

/* Note: From MSP430F5438x Errata sheet
 * USCI35 - USCI Module Function
 * Violation of setup and hold times for (repeated) start in I2C master mode
 * In I2C master mode, the setup and hold times for a (repeated) START, tSU,STA
 * and tHD,STA respectively, can be violated if SCL clock frequency is greater
 * than 50kHz in standard mode (100kbps). As a result, a slave can receive incorrect
 * data or the I2C bus can be stalled due to clock stretching by the slave.
 *
 * Workaround:
 * If using repeated start, ensure SCL clock frequencies is <50kHz in I2C standard
 * mode (100kbps)
 *
 * The implication being that clock frequencies from 50Khz to 100Khz are not reliable.
 */
#define SCL_CLOCK_DIV 50					// SCL clock divider

#if 0
// Linux i2c interface. Should we make it look more like this?
struct i2c_msg {
  __u16 addr;
  __u16 flags;
#define I2C_M_TEN		0x0010
#define I2C_M_RD		0x0001
#define I2C_M_NOSTART		0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
#define I2C_M_RECV_LEN		0x0400
  __u16 len;
  __u8 * buf;
};
int i2c_transfer (	struct i2c_adapter * adap,
 	struct i2c_msg * msgs,
 	int num);
Note: In linux all i2c_msg start with START, STOP send when all msgs complete
#endif

struct msp430_i2c {
	struct i2c_adapter i2c;
} msp430_i2c;

static int msp430_i2c_write_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags);
static int msp430_i2c_read_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags);


/*
 *   Initialization of the I2C Module.
 *   Which i2c interface is determined at compile time.
 */
int msp430_i2c_init(u8 channel, struct i2c_adapter **i2c)
{
	// we only support one i2c channel on MSP430 */
	assert(channel == 0);

	gpio_request(I2C_SCL, PIN_SPECIAL);
	gpio_request(I2C_SDA, PIN_SPECIAL);

	// Recommended initialisation steps of I2C module as shown in User Guide:
	UCxnCTL1 |= UCSWRST;                    // Enable SW reset
	UCxnCTL0 = UCMST | UCMODE_3 | UCSYNC;   // I2C Master, synchronous mode
	UCxnCTL1 = UCSSEL_2 | UCTR | UCSWRST;   // Use SMCLK, TX mode, keep SW reset
	UCxnBR0 = SCL_CLOCK_DIV;                // fSCL = SMCLK/N = ~400kHz
	UCxnBR1 = 0;
	UCxnI2COA = 0x01A5;                     // own address (no general call)
	UCxnIE = 0;								// disable all interrupts
	UCxnCTL1 &= ~UCSWRST;                   // Clear SW reset, resume operation

	// if bus not free generate a manual clock pulse.
	if (UCxnSTAT & UCBBUSY)
	{
		gpio_request(I2C_SCL, PIN_GPIO | PIN_OUTPUT | PIN_INIT_LOW | PIN_REDEFINE);
		gpio_request(I2C_SCL, PIN_SPECIAL | PIN_REDEFINE);
		assert ((UCxnSTAT & UCBBUSY) == 0);
	}

	msp430_i2c.i2c.read_bytes = msp430_i2c_read_bytes;
	msp430_i2c.i2c.write_bytes = msp430_i2c_write_bytes;
	*i2c = (struct i2c_adapter*)&msp430_i2c;

	return 0;
}

/*
 * Write bytes to specified device - optional start and stop
 * First byte has to be preloaded for start to complete
 */
static int msp430_i2c_write_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags)
{
	int result = -EIO;
	unsigned int gie = __get_SR_register() & GIE; //Store current GIE state

	__disable_interrupt();              	// Make this operation atomic

	if (count == 0)							// no data but may want to stop
		goto no_data;

	if (!(flags & I2C_NO_START))
	{
		UCxnI2CSA  = i2c_addr;        		// set Slave Address

		UCxnIFG = 0;						// clear all interrupt flags
		UCxnCTL1 |= UCTR;                   // UCTR=1 => Transmit Mode (R/W bit = 0)
		UCxnCTL1 |= UCTXSTT;				// Transmit start

		UCxnTXBUF = *data++;				// preload first byte
		count--;

		while (UCxnCTL1 & UCTXSTT);			// wait for START to complete
		if (UCxnIFG & UCNACKIFG)			// bail if NACK'd
			goto error;
	}

	while (count--)
	{
		while ((UCxnIFG & UCTXIFG) == 0) {	// Wait for ok to send next byte
			if (UCxnIFG & UCNACKIFG)		// see if last byte was NACK'd
				goto error;
		}
		UCxnTXBUF = *data++;				// send the next byte
	}

	while ((UCxnIFG & UCTXIFG) == 0);		// Wait for last byte to clear

no_data:
	result = 0;

	// suppress stop if requested
	if (!(flags & I2C_NO_STOP))
	{
error:
		UCxnCTL1 |= UCTXSTP;				// Transmit Stop
		while (UCxnCTL1 & UCTXSTP);      	// Ensure stop condition got sent
	}

	__bis_SR_register(gie);             	// Restore original GIE state

	return result;
}

/*
 * Read bytes from specified device - optional start and stop
 * There is an issue with receiving a single byte. We must
 * issue the stop before we even get the byte. Suspect emptying the
 * rx data buffer triggers the next read operation.
 */
static int msp430_i2c_read_bytes(struct i2c_adapter *i2c, u8 i2c_addr, u8 *data, u8 count, u8 flags)
{
	int result = -EIO;
	int remaining = count;
	int send_stop;
	int stop_sent = 0;
	unsigned int gie = __get_SR_register() & GIE; // store current GIE state

	__disable_interrupt();              	// Make this operation atomic

	send_stop = (!(flags & I2C_NO_STOP) && (count == 1));

	if (count == 0)
		goto no_data;

	if (!(flags & I2C_NO_START))
	{
		UCxnI2CSA  = i2c_addr;         		// set Slave Address

		UCxnIFG = 0;						// clear interrupt flags
		UCxnCTL1 &= ~UCTR;                  // UCTR=0 => Receive Mode (R/W bit = 1)
		UCxnCTL1 |= UCTXSTT;				// Transmit start

		while (UCxnCTL1 & UCTXSTT);			// wait for START to complete
		if (UCxnIFG & UCNACKIFG)			// bail if NACK'd
			goto error;

		// special case handling of single byte read in case timing critical
		if (send_stop) {
			UCxnCTL1 |= UCTXSTP;			// generate stop
			while (UCxnCTL1 & UCTXSTP);     // Ensure stop condition got sent

			while ((UCxnIFG & UCRXIFG) == 0);	// Wait for data ready
			*data++ = UCxnRXBUF;				// read the byte (allows collection of next)
			remaining--;
			stop_sent = 1;
		}
	}

	while (remaining)
	{
		remaining--;
		if (remaining == 0 && !(flags & I2C_NO_STOP)) {
			UCxnCTL1 |= UCTXSTP;			// generate stop
			while(UCxnCTL1 & UCTXSTP);      // Ensure stop condition got sent
			stop_sent = 1;
		}
		while ((UCxnIFG & UCRXIFG) == 0);	// Wait for data ready
		*data++ = UCxnRXBUF;				// read the byte (allows collection of next)
	}

no_data:
	result = 0;

	// suppress stop if requested or not already sent
	if (!(flags & I2C_NO_STOP) && !stop_sent)
	{
error:
		UCxnCTL1 |= UCTXSTP;				// Transmit Stop
		while(UCxnCTL1 & UCTXSTP);          // Ensure stop condition got sent
	}
	__bis_SR_register(gie);             	// Restore original GIE state

	return result;
}

