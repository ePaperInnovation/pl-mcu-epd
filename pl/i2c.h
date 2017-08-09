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
 * i2c.h -- I2C interface abstraction layer
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_I2C_H
#define INCLUDE_PL_I2C_H 1

#include <stdint.h>

/**
   @file pl/i2c.h

   Abstract interface to use an I2C bus master.
*/

/** Transation flags */
enum pl_i2c_flags {
	PL_I2C_NO_STOP  = 1 << 0,       /**< Do not generate stop bit */
	PL_I2C_NO_START = 1 << 1,       /**< Do not generate start bit */
};

/** Interface to be populated with concrete implementations */
struct pl_i2c {
	/**
	   read some data on the I2C bus
	   @param[in] i2c this pl_i2c instance
	   @param[in] addr 8-bit I2C address
	   @param[out] data buffer to receive the data being read
	   @param[in] count number of bytes to read
	   @param[in] flags flags bitmask using pl_i2c_flags
	   @return -1 if error, 0 otherwise
	 */
	int (*read)(struct pl_i2c *i2c, uint8_t addr,
		    uint8_t *data, uint8_t count, uint8_t flags);

	/**
	   write some data on the I2C bus
	   @param[in] i2c this pl_i2c instance
	   @param[in] addr 8-bit I2C address
	   @param[in] data buffer with data to be written
	   @param[in] count number of bytes to write
	   @param[in] flags flags bitmask using pl_i2c_flags
	   @return -1 if error, 0 otherwise
	 */
	int (*write)(struct pl_i2c *i2c, uint8_t addr,
		     const uint8_t *data, uint8_t count, uint8_t flags);

	/**
	   free the resources associated with this instance
	   @param i2c this pl_i2c instance
	 */
	void (*free)(struct pl_i2c *i2c);

	/**
	   private data specific to this instance
	 */
	void *priv;
};

/** Read an 8-bit register on the I2C bus
    @param[in] i2c pl_i2c instance
    @param[in] addr I2C address
    @param[in] reg register address
    @param[out] data pointer to store the 8-bit register value
    @return -1 if error, 0 otherwise
 */
extern int pl_i2c_reg_read_8(struct pl_i2c *i2c, uint8_t addr,
			     uint8_t reg, uint8_t *data);

/** Write an 8-bit register on the I2C bus
    @param[in] i2c pl_i2c instance
    @param[in] addr I2C address
    @param[in] reg register address
    @param[in] data 8-bit register value
    @return -1 if error, 0 otherwise
 */
extern int pl_i2c_reg_write_8(struct pl_i2c *i2c, uint8_t addr,
			      uint8_t reg, uint8_t data);

/** Read a 16-bit big-endian register on the I2C bus
    @param[in] i2c pl_i2c instance
    @param[in] addr I2C address
    @param[in] reg register address
    @param[out] data pointer to store the 16-bit register value
    @return -1 if error, 0 otherwise
 */
extern int pl_i2c_reg_read_16be(struct pl_i2c *i2c, uint8_t addr,
				uint8_t reg, uint16_t *data);

/** Write a 16-bit big-endian register on the I2C bus
    @param[in] i2c pl_i2c instance
    @param[in] addr I2C address
    @param[in] reg register address
    @param[in] data 16-bit register value
    @return -1 if error, 0 otherwise
 */
extern int pl_i2c_reg_write_16be(struct pl_i2c *i2c, uint8_t addr,
				 uint8_t reg, uint16_t data);

#endif /* INCLUDE_PL_I2C_H */
