/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * epson-if.c -- Epson interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "platform.h"
#include "types.h"
#include "assert.h"
#include "msp430-spi.h"
#include "msp430-gpio.h"

#define CONFIG_PLAT_RUDDOCK2	1

// Optional pins used in SPI interface
// HRDY indicates controller is ready
#define SPI_HRDY_USED		0	// HRDY pin is used
#define SPI_HRDY_SELECT		1	// HRDY pin only driven when selected
// HDC required by the 524 controller, optional on others
#define SPI_HDC_USED		1	// HDC pin is used
#define SPI_RESET_USED		1	// Reset Pin on Epson used

#if CONFIG_PLAT_RUDDOCK2
// Remaining Epson interface pins
#define	EPSON_HDC		GPIO(1,3)
#define	EPSON_HRDY		GPIO(2,7)
#define EPSON_RESET		GPIO(5,0)

#endif


static screen_t screen;
static int busy;


void epsonif_init_hdc(void)
{
#if SPI_HDC_USED
	gpio_request(EPSON_HDC, PIN_GPIO | PIN_OUTPUT);
#endif
}

void epsonif_init_reset(void)
{
#if SPI_RESET_USED
	gpio_request(EPSON_RESET, PIN_GPIO | PIN_OUTPUT | PIN_INIT_HIGH);
#endif
}

void epsonif_init_hrdy(void)
{
#if SPI_HRDY_USED
	gpio_request(EPSON_HRDY, PIN_GPIO | PIN_INPUT);
#endif
}

int epsonif_read_hrdy(void)
{
#if SPI_HRDY_USED
	return gpio_get_value(EPSON_HRDY);
#else
	return 0;
#endif
}

/* Before using the spi interface you have to claim it and tell it what screen id
 * you will be talking to. This is to avoid having to pass screen everywhere.
 * You get the current screen id back. This is to allow nesting of spi access.
 */
int epsonif_claim(int spi_channel, screen_t screen_id, screen_t *previous)
{
	*previous = screen;

	if (spi_channel != 0)
		return -EACCES;

	screen = screen_id;
	busy++;

	return 0;
}

/* Indicate the spi interface is no longer in use at a higher level.
 * Restores the previous screen selection, if there was one
 */
int epsonif_release(int spi_channel, screen_t previous)
{
	if (busy <= 0)
		return -EINVAL;

	screen = previous;
	busy--;

	return 0;
}

void epsonif_set_command()
{
#if SPI_HDC_USED
	gpio_set_value_lo(EPSON_HDC);
#endif
}

void epsonif_set_data()
{
#if SPI_HDC_USED
	gpio_set_value_hi(EPSON_HDC);
#endif
}

void epsonif_assert_reset(void)
{
#if	SPI_RESET_USED
	gpio_set_value_lo(EPSON_RESET);
#endif
}
void epsonif_negate_reset(void)
{
#if	SPI_RESET_USED
	gpio_set_value_hi(EPSON_RESET);
#endif
}

int epsonif_select_epson(void)
{
	assert(screen > 0);

	gpio_set_value_lo(screen);
	return 0;
}

int epsonif_deselect_epson(void)
{
	assert(screen > 0);

	gpio_set_value_hi(screen);
	return 0;
}

void epsonif_init(int spi_channel, u16 divisor)
{
	spi_init(spi_channel, divisor);
	epsonif_init_hdc();
	epsonif_init_hrdy();
	epsonif_init_reset();
	epsonif_assert_reset();
	mdelay(4);
	epsonif_negate_reset();

	screen = -1;
	busy = 0;
}
