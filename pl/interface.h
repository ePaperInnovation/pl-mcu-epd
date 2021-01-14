/*
  Copyright (C) 2017 Plastic Logic

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
 * interface.h -- Epson interface abstraction layer
 *
 * Authors:
 *   Robert Pohlink <robert.pohlink@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_INTERFACE_H
#define INCLUDE_PL_INTERFACE_H 1

#include <stdint.h>
#include <pl/endian.h>

struct pl_gpio;

struct spi_metadata {
	uint8_t channel;  // SPI channel number
	uint8_t mode;     // current SPI mode
	uint8_t bpw;      // current SPI bits per word setting
	uint32_t msh;     // current SPI max speed setting in Hz
};

struct pl_interface
{
  int cs_gpio; 		// chip select gpio
  int (*read)(uint8_t *buff, uint16_t size);
  int (*write)(uint8_t *buff, uint16_t size);
  int (*set_cs)(uint8_t cs);

  struct spi_metadata *mSpi;
};

int spi_init(struct pl_gpio *gpio, uint8_t spi_channel, uint16_t divisor, struct pl_interface *iface);
int parallel_init(struct pl_gpio *gpio, struct pl_interface *iface);
#endif
