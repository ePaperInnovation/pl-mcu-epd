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

#include <pl/interface.h>
#include <pl/endian.h>
#include <pl/gpio.h>
#include <msp430/msp430-parallel.h>
#include <msp430/msp430-spi.h>

struct pl_interface;
struct pl_gpio;

extern int spi_init(struct pl_gpio *gpio, uint8_t spi_channel, uint16_t divisor, struct pl_interface *iface){
	return msp430_spi_init(gpio, spi_channel, divisor, iface);
}

extern int parallel_init(struct pl_gpio *gpio, struct pl_interface *iface){
	return msp430_parallel_init(gpio, iface);
}
