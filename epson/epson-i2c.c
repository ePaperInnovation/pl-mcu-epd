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

#include <pl/i2c.h>
#include "assert.h"
#include "epson-epdc.h"
#include "epson-s1d135xx.h"

#define LOG_TAG "epson-i2c"
#include "utils.h"

enum s1d135xx_i2c_reg {
	S1D135XX_I2C_REG_STAT             = 0x0218,
	S1D135XX_I2C_REG_CMD              = 0x021A,
	S1D135XX_I2C_REG_RD               = 0x021C,
	S1D135XX_I2C_REG_WD               = 0x021E,
};

enum s1d135xx_i2c_status {
	/* Busy */
	S1D135XX_I2C_STAT_GO              = (1 << 0),
	/* ACK(0)/NAK(1) from slave on write */
	S1D135XX_I2C_STAT_RX_NAK          = (1 << 1),
	/* I2C Busy when START detected (0 on STOP) */
	S1D135XX_I2C_STAT_BUSY            = (1 << 2),
	/* Transfer in progress */
	S1D135XX_I2C_STAT_TIP             = (1 << 3),
	/* Transfer/Stop command issued when bus inactive */
	S1D135XX_I2C_STAT_ERROR           = (1 << 6),
	/* I2C controller reset */
	S1D135XX_I2C_STAT_RESET           = (1 << 15),
};

enum s1d135xx_i2c_cmd {
	/* Initiate i2c transfer */
	S1D135XX_I2C_CMD_GO               = (1 << 0),
	/* Read (1) or Write (0) to slave */
	S1D135XX_I2C_CMD_READ             = (1 << 1),
	/* Ack (0) or NAK (1) to slave on read */
	S1D135XX_I2C_CMD_TX_NAK           = (1 << 2),
	/* Generate Start/Stop (1) */
	S1D135XX_I2C_CMD_GEN              = (1 << 4),
	/* Select Start (1) or Stop (0) */
	S1D135XX_I2C_CMD_START            = (1 << 5),
	/* Send no Data (1) */
	S1D135XX_I2C_CMD_NO_DATA          = (1 << 6),
};

static int epson_s1d135xx_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr,
				   uint8_t *data, uint8_t count,
				   uint8_t flags);
static int epson_s1d135xx_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
				    const uint8_t *data, uint8_t count,
				    uint8_t flags);
static int s1d135xx_i2c_send_addr(struct s1d135xx *p, uint8_t i2c_addr,
				  uint8_t read);
static int s1d135xx_i2c_poll(struct s1d135xx *p, int check_nak);

/*
 *   Initialization of the I2C Module
 */
int epson_i2c_init(struct s1d135xx *p, struct pl_i2c *i2c,
		   enum epson_epdc_ref ref)
{
	if (epson_epdc_early_init(p, ref))
		return -1;

	i2c->read = epson_s1d135xx_i2c_read;
	i2c->write = epson_s1d135xx_i2c_write;
	i2c->priv = p;

	return 0;
}

static int epson_s1d135xx_i2c_read(struct pl_i2c *i2c, uint8_t i2c_addr,
				   uint8_t *data, uint8_t count, uint8_t flags)
{
	struct s1d135xx *p = i2c->priv;

	if (!(flags & PL_I2C_NO_START))
		if (s1d135xx_i2c_send_addr(p, i2c_addr, 1))
			return -1;

	while (count--) {
		uint8_t cmd;

		if (!count && !(flags & PL_I2C_NO_STOP))
			cmd = (S1D135XX_I2C_CMD_GO | S1D135XX_I2C_CMD_READ |
			       S1D135XX_I2C_CMD_GEN | S1D135XX_I2C_CMD_TX_NAK);
		else
			cmd = S1D135XX_I2C_CMD_GO | S1D135XX_I2C_CMD_READ;

		s1d135xx_write_reg(p, S1D135XX_I2C_REG_CMD, cmd);

		if (s1d135xx_i2c_poll(p, 0))
			return -1;

		*data++ = s1d135xx_read_reg(p, S1D135XX_I2C_REG_RD);
	}

	return 0;
}

static int epson_s1d135xx_i2c_write(struct pl_i2c *i2c, uint8_t i2c_addr,
				    const uint8_t *data, uint8_t count,
				    uint8_t flags)
{
	struct s1d135xx *p = i2c->priv;

	if (!(flags & PL_I2C_NO_START))
		if (s1d135xx_i2c_send_addr(p, i2c_addr, 0))
			return -1;

	while (count--) {
		uint8_t cmd;

		if (!count && !(flags & PL_I2C_NO_STOP))
			cmd = S1D135XX_I2C_CMD_GO | S1D135XX_I2C_CMD_GEN;
		else
			cmd = S1D135XX_I2C_CMD_GO;

		s1d135xx_write_reg(p, S1D135XX_I2C_REG_WD, *data++);
		s1d135xx_write_reg(p, S1D135XX_I2C_REG_CMD, cmd);

		if (s1d135xx_i2c_poll(p, 1))
			return -1;
	}

	return 0;
}

static int s1d135xx_i2c_send_addr(struct s1d135xx *p, uint8_t i2c_addr,
				  uint8_t read)
{
	s1d135xx_write_reg(p, S1D135XX_I2C_REG_WD, ((i2c_addr) << 1) | read);
	s1d135xx_write_reg(p, S1D135XX_I2C_REG_CMD, (S1D135XX_I2C_CMD_START |
						     S1D135XX_I2C_CMD_GEN |
						     S1D135XX_I2C_CMD_GO));

	return s1d135xx_i2c_poll(p, 1);
}

static int s1d135xx_i2c_poll(struct s1d135xx *p, int check_nak)
{
	uint16_t status;
	unsigned i = 50;

	while (--i) {
		status = s1d135xx_read_reg(p, S1D135XX_I2C_REG_STAT);

		if (!(status & S1D135XX_I2C_STAT_GO))
			break;

		mdelay(1);
	}

	if (!i)
		LOG("TIMEOUT");
	else  if (status & S1D135XX_I2C_STAT_ERROR)
		LOG("ERROR");
	else if (check_nak && (status & S1D135XX_I2C_STAT_RX_NAK))
		LOG("NAK");
	else
		return 0;

	return -1;
}
