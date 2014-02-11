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
 * epson-if.c -- Epson interface driver
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/gpio.h>
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
#define	EPSON_HDC               MSP430_GPIO(1,3)
#define	EPSON_HRDY              MSP430_GPIO(2,7)
#define EPSON_RESET             MSP430_GPIO(5,0)

#endif

static struct pl_gpio *epsonif_gpio = NULL;
static screen_t screen;
static int busy;

int epsonif_init_reset(void)
{
#if SPI_RESET_USED
	return epsonif_gpio->config(EPSON_RESET,
				    PL_GPIO_OUTPUT | PL_GPIO_INIT_H);
#endif
}

int epsonif_read_hrdy(void)
{
#if SPI_HRDY_USED
	return epsonif_gpio->get(EPSON_HRDY);
#else
	return 0;
#endif
}

/* Before using the spi interface you have to claim it and tell it what screen
 * id you will be talking to. This is to avoid having to pass screen
 * everywhere.  You get the current screen id back. This is to allow nesting of
 * spi access.
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

/* Indicate the spi interface is no longer in use at a higher level.  Restores
 * the previous screen selection, if there was one.
 */
int epsonif_release(int spi_channel, screen_t previous)
{
	if (busy <= 0)
		return -EINVAL;

	screen = previous;
	busy--;

	return 0;
}

void epsonif_set_command(void)
{
#if SPI_HDC_USED
	epsonif_gpio->set(EPSON_HDC, 0);
#endif
}

void epsonif_set_data(void)
{
#if SPI_HDC_USED
	epsonif_gpio->set(EPSON_HDC, 1);
#endif
}

void epsonif_assert_reset(void)
{
#if SPI_RESET_USED
	epsonif_gpio->set(EPSON_RESET, 0);
#endif
}
void epsonif_negate_reset(void)
{
#if SPI_RESET_USED
	epsonif_gpio->set(EPSON_RESET, 1);
#endif
}

void epsonif_select_epson(void)
{
	assert(screen > 0);

	epsonif_gpio->set(screen, 0);
}

void epsonif_deselect_epson(void)
{
	assert(screen > 0);

	epsonif_gpio->set(screen, 1);
}

int epsonif_init(struct pl_gpio *gpio, int spi_channel, u16 divisor)
{
	static const struct pl_gpio_config gpios[] = {
#if SPI_HDC_USED
		{ EPSON_HDC,  PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
#endif
#if SPI_HRDY_USED
		{ EPSON_HRDY, PL_GPIO_INPUT },
#endif
	};

	epsonif_gpio = gpio;
	spi_init(gpio, spi_channel, divisor);

	if (pl_gpio_config_list(gpio, gpios, ARRAY_SIZE(gpios)))
		return -1;

	if (epsonif_init_reset())
		return -1;

	epsonif_assert_reset();
	mdelay(4);
	epsonif_negate_reset();

	screen = -1;
	busy = 0;

	return 0;
}
