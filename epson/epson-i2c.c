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
 * epson-i2c.c -- Epson i2c master Controller driver
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <msp430.h>
#include "types.h"
#include "assert.h"
#include "i2c.h"
#include "S1D135xx.h"
#include "epson-i2c.h"
#include "epson-cmd.h"

// registers are common to S1D13524, S1D13541
#define I2C_CLK_CONF_REG 		0x001A // I2C clock Config register
#define I2C_STAT_REG			0x0218 // I2C status register
#define I2C_CMD_REG				0x021A // I2C command register
#define I2C_RD_REG				0x021C // I2C read data register
#define I2C_WD_REG				0x021E // I2C write data register

// Status/Configuration register bits
#define	I2C_STAT_GO				BIT0	// Busy
#define	I2C_STAT_NAK			BIT1	// ACK(0)/NAK(1) from slave on write
#define I2C_STAT_BUSY			BIT2	// I2C Busy when START detected (0 on STOP)
#define	I2C_STAT_TIP			BIT3	// Transfer in progress
#define I2C_STAT_ERROR			BIT6	// Transfer/Stop command issued when bus inactive
#define I2C_STAT_RESET			BITF	// I2C controller reset
#define I2C_STAT_SAMPLE_AT(_x_)	(((_x_) & 0x7) << 8) // sample point for the read data/ack bit

// Command register bits
#define	I2C_CMD_GO				BIT0	// initiate i2c transfer
#define	I2C_CMD_READ			BIT1	// Read (1) or Write (0) to slave
#define	I2C_CMD_WRITE			0
#define	I2C_CMD_TX_NAK			BIT2	// Ack (0) or NAK (1) to slave on read
#define I2C_CMD_GEN_STARTSTOP	BIT4	// Generate Start/Stop (1)
#define	I2C_CMD_SEL_STARTSTOP	BIT5	// Select Start (1) or Stop (0)
#define I2C_CMD_NO_DATA			BIT6	// Send no Data (1)
#define	I2C_CMD_ACTION(_x_)		((_x_) << 4)
#define	I2C_DO_DATA_TRANSFER		0x00	// Transfer data - in direction specified
#define	I2C_DO_DATA_TRANSFER_STOP	0x01	// Transfer data then generate STOP
#define	I2C_DO_START_TRANSFER		0x03	// Generate START then transfer data
#define	I2C_DO_STOP					0x05	// STOP
#define	I2C_DO_START				0x07	// START

// Hybrid commands for the actions we need
#define	I2C_SELECT_DEVICE		(I2C_CMD_ACTION(I2C_DO_START_TRANSFER) | I2C_CMD_WRITE)
#define	I2C_WRITE_BYTE			(I2C_CMD_ACTION(I2C_DO_DATA_TRANSFER)  | I2C_CMD_WRITE)
#define	I2C_READ_BYTE			(I2C_CMD_ACTION(I2C_DO_DATA_TRANSFER)  | I2C_CMD_READ)
#define	I2C_READ_LAST_BYTE		(I2C_CMD_ACTION(I2C_DO_DATA_TRANSFER)  | I2C_CMD_READ | I2C_CMD_TX_NAK)
#define	I2C_STOP				(I2C_CMD_ACTION(I2C_DO_STOP))

struct epson_i2c {
	struct i2c_adapter i2c;
	screen_t screen;
	uint8_t busy;
};

static int epson_i2c_read_bytes(struct i2c_adapter *i2c, uint8_t i2c_addr,
				uint8_t *data, uint8_t count, uint8_t flags);
static int epson_i2c_write_bytes(struct i2c_adapter *i2c, uint8_t i2c_addr,
				 const uint8_t *data, uint8_t count,
				 uint8_t flags);

static struct epson_i2c epson_i2c;

/*
 *   Initialization of the I2C Module
 */
int epson_i2c_init(struct s1d135xx *epson, struct i2c_adapter *i2c)
{
	screen_t previous;

	assert(epson);
	assert(i2c);

	epson_i2c.i2c.read_bytes = epson_i2c_read_bytes;
	epson_i2c.i2c.write_bytes = epson_i2c_write_bytes;
	epson_i2c.screen = epson->screen;
	epson_i2c.busy = 0;

	epsonif_claim(0,epson->screen, &previous);

	/* reset the controller */
	epson_reg_write(I2C_STAT_REG, I2C_STAT_RESET);
	epson_wait_for_idle();
	epson_reg_write(I2C_STAT_REG, I2C_STAT_SAMPLE_AT(3));

	/* set I2C clock divider (0..15) */
	epson_reg_write(I2C_CLK_CONF_REG, 0x0007);

	epsonif_release(0, previous);

	i2c = &epson_i2c.i2c;

	return 0;
}

/* Send a command to the Epson i2c unit, wait for it to finish
 * and check the outcome.
 */
static int epson_i2c_command(int cmd, int flags)
{
	int result = 0;
	uint16_t stat;

	epson_reg_write(I2C_CMD_REG, (cmd | I2C_CMD_GO));

	// wait for the command to complete
	do {
		epson_reg_read(I2C_STAT_REG, &stat);
	} while(stat & I2C_STAT_GO);

	// check for requested flags and general error response
	if(stat & (flags | I2C_STAT_ERROR))
		result = -EIO;

	return result;
}

/*
 * Write bytes to specified device - optional start and stop
 */
static int epson_i2c_write_bytes(struct i2c_adapter *i2c, uint8_t i2c_addr,
				 const uint8_t *data, uint8_t count,
				 uint8_t flags)
{
	screen_t previous;
	int result = -EIO;
	struct epson_i2c *p = (struct epson_i2c*)i2c;

	epsonif_claim(0, p->screen, &previous);

	if (count == 0)
		goto no_data;

	if (!(flags & I2C_NO_START))
	{
		// send the i2c address of the slave
		epson_reg_write(I2C_WD_REG, (i2c_addr << 1) | 0);
		if (epson_i2c_command(I2C_SELECT_DEVICE, I2C_STAT_NAK) < 0)
			goto error;
	}

	while (count--)
	{
		// write the bytes of data out.
		epson_reg_write(I2C_WD_REG, *data++);
		if (epson_i2c_command(I2C_WRITE_BYTE, I2C_STAT_NAK) < 0)
			goto error;
	}

no_data:
	result = 0;

	// dont send stop if requested
	if (!(flags & I2C_NO_STOP))
	{
error:
		epson_i2c_command(I2C_STOP, 0);
	}

	epsonif_release(0, previous);

	return result;
}

/*
 * Read bytes from specified device - optional start and stop
 */
static int epson_i2c_read_bytes(struct i2c_adapter *i2c, uint8_t i2c_addr, uint8_t *data,
				uint8_t count, uint8_t flags)
{
	screen_t previous;
	uint16_t stat;
	uint16_t cmd;
	int result = -EIO;
	struct epson_i2c *p = (struct epson_i2c*)i2c;

	epsonif_claim(0, p->screen, &previous);

	if (count == 0)
		goto no_data;

	if (!(flags & I2C_NO_START))
	{
		// send the i2c address of the slave
		epson_reg_write(I2C_WD_REG, (i2c_addr << 1) | 1);
		if (epson_i2c_command(I2C_SELECT_DEVICE, I2C_STAT_NAK) < 0)
			goto error;
	}

	// read bytes from the slave, send NAK when reading last byte
	while (count)
	{
		count--;
		cmd = (count ? I2C_READ_BYTE : I2C_READ_LAST_BYTE);
		if (epson_i2c_command(cmd, I2C_STAT_NAK) < 0)
			goto error;
		epson_reg_read(I2C_RD_REG, &stat);
		*data++ = (uint8_t)stat;
	}

no_data:
	result = 0;

	// dont send stop if requested
	if (!(flags & I2C_NO_STOP))
	{
error:
		epson_i2c_command(I2C_STOP, 0);
	}

	epsonif_release(0, previous);

	return result;
}

