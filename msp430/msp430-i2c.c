/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/gpio.h>
#include <pl/i2c.h>
#include <msp430.h>
#include <stdint.h>
#include "types.h"
#include "assert.h"
#include "msp430-defs.h"
#include "msp430-gpio.h"

#define CONFIG_PLAT_RUDDOCK2	1

#if CONFIG_PLAT_RUDDOCK2
#define USCI_UNIT	B
#define USCI_CHAN	1
// Pin definitions for this unit
#define	I2C_SCL         MSP430_GPIO(5,4)
#define I2C_SDA         MSP430_GPIO(3,7)

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

static int msp430_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
			    const uint8_t *data, uint8_t count, uint8_t flags);
static int msp430_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr,
			   uint8_t *data, uint8_t count, uint8_t flags);

/*
 *   Initialization of the I2C Module.
 *   Which i2c interface is determined at compile time.
 */
int msp430_i2c_init(struct pl_gpio *gpio, u8 channel, struct pl_i2c *i2c)
{
	static const struct pl_gpio_config gpios[] = {
		{ I2C_SCL, PL_GPIO_SPECIAL },
		{ I2C_SDA, PL_GPIO_SPECIAL },
	};

	/* we only support one i2c channel on MSP430 */
	assert(channel == 0);

	if (pl_gpio_config_list(gpio, gpios, ARRAY_SIZE(gpios)))
		return -1;

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
	if (UCxnSTAT & UCBBUSY)	{
		if (gpio->config(I2C_SCL, PL_GPIO_OUTPUT | PL_GPIO_INIT_L))
			return -1;

		if (gpio->config(I2C_SCL, PL_GPIO_SPECIAL))
			return -1;

		assert(!(UCxnSTAT & UCBBUSY));
	}

	i2c->read = msp430_i2c_read;
	i2c->write = msp430_i2c_write;

	return 0;
}

/*
 * Write bytes to specified device - optional start and stop
 * First byte has to be preloaded for start to complete
 */
static int msp430_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
			    const uint8_t *data, uint8_t count, uint8_t flags)
{
	int result = -EIO;
	unsigned int gie = __get_SR_register() & GIE; //Store current GIE state

	__disable_interrupt();              	// Make this operation atomic

	if (count == 0)							// no data but may want to stop
		goto no_data;

	if (!(flags & PL_I2C_NO_START))
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
	if (!(flags & PL_I2C_NO_STOP))
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
static int msp430_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr, uint8_t *data,
			   uint8_t count, uint8_t flags)
{
	int result = -EIO;
	int remaining = count;
	int send_stop;
	int stop_sent = 0;
	unsigned int gie = __get_SR_register() & GIE; // store current GIE state

	__disable_interrupt();              	// Make this operation atomic

	send_stop = (!(flags & PL_I2C_NO_STOP) && (count == 1));

	if (count == 0)
		goto no_data;

	if (!(flags & PL_I2C_NO_START))
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
		if (remaining == 0 && !(flags & PL_I2C_NO_STOP)) {
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
	if (!(flags & PL_I2C_NO_STOP) && !stop_sent)
	{
error:
		UCxnCTL1 |= UCTXSTP;				// Transmit Stop
		while(UCxnCTL1 & UCTXSTP);          // Ensure stop condition got sent
	}
	__bis_SR_register(gie);             	// Restore original GIE state

	return result;
}

