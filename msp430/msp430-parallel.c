/*
  Plastic Logic EPD project on MSP430

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
 * msp430-parallel.c -- MSP430 parallel interface driver
 *
 * Authors: Robert Pohlink <robert.pohlink@plasticlogic.com>
 *
 */

#include <pl/gpio.h>
#include <pl/interface.h>
#include <msp430.h>
#include "utils.h"
#include "assert.h"
#include "msp430-defs.h"
#include "msp430-parallel.h"
#include "msp430-gpio.h"

#define CONFIG_PLAT_RUDDOCK2	1

#define	READ_STROBE             MSP430_GPIO(3,4)
#define	WRITE_STROBE            MSP430_GPIO(3,0)

#if CONFIG_PLAT_RUDDOCK2
#define USCI_UNIT	A
#define USCI_CHAN	0
#endif

int msp430_parallel_read_bytes(uint8_t *buff, uint8_t size);
int msp430_parallel_write_bytes(uint8_t *buff, uint8_t size);

int msp430_parallel_init(struct pl_gpio *gpio, struct pl_interface *iface)
{
	static const struct pl_gpio_config gpios[] = {
		{ READ_STROBE,   PL_GPIO_OUTPUT |  PL_GPIO_INIT_H },
		{ WRITE_STROBE,  PL_GPIO_OUTPUT |  PL_GPIO_INIT_H },
	};

	UCxnCTL1 |= UCSWRST;					// Put state machine in reset

	if (pl_gpio_config_list(gpio, gpios, ARRAY_SIZE(gpios)))
		return -1;
	iface->write = msp430_parallel_write_bytes;
	iface->read = msp430_parallel_read_bytes;
	return 0;
}

// Higher level read register command needs to know not to ditch first word
// in parallel port as its valid data.
int msp430_parallel_read_bytes(uint8_t *buff, uint8_t size)
{
	assert((size & 1) == 0);

	// define ports as input
	P6DIR = 0x00;
	P4DIR = 0x00;

	do {
		msp430_gpio_set(READ_STROBE, 0);
		__no_operation();
		*buff++ = P6IN;
		*buff++ = P4IN;
		msp430_gpio_set(READ_STROBE, 1);
		__no_operation();
	} while (size -= 2);
	return 0;
}
int msp430_parallel_write_bytes(uint8_t *buff, uint8_t size)
{
	assert((size & 1) == 0);

	// define ports as output
	P6DIR = 0xff;
	P4DIR = 0xff;
#if 0
	{
		uint8_t bit = 1;
		while (bit)
		{
			P4OUT = bit;
			P6OUT = bit;
			bit <<= 1;
		}
	}
#endif
	do {
		msp430_gpio_set(WRITE_STROBE, 0);
		P6OUT = *buff++;
		P4OUT = *buff++;
		msp430_gpio_set(WRITE_STROBE, 1);
		__no_operation();
	} while (size -= 2);
	return 0;
}






