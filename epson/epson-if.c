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
#include "plat-epson.h"
#include "types.h"
#include "assert.h"
#include "msp430-spi.h"
#include "msp430-gpio.h"

static struct pl_gpio *g_epsonif_gpio = NULL;
static const struct epson_gpio_config *g_epson_gpio = NULL;
static screen_t screen;
static int busy;

int epsonif_init_reset(void)
{
	if (g_epson_gpio->reset == PL_GPIO_NONE)
		return 0;

	return g_epsonif_gpio->config(g_epson_gpio->reset,
				      PL_GPIO_OUTPUT | PL_GPIO_INIT_H);
}

int epsonif_read_hrdy(void)
{
	if (g_epson_gpio->hrdy == PL_GPIO_NONE)
		return 0; /* ToDo: read SPI register if no GPIO */

	return g_epsonif_gpio->get(g_epson_gpio->hrdy);
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
	if (g_epson_gpio->hdc != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_epson_gpio->hdc, 0);
}

void epsonif_set_data(void)
{
	if (g_epson_gpio->hdc != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_epson_gpio->hdc, 1);
}

void epsonif_assert_reset(void)
{
	if (g_epson_gpio->reset != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_epson_gpio->reset, 0);
}
void epsonif_negate_reset(void)
{
	if (g_epson_gpio->reset != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_epson_gpio->reset, 1);
}

void epsonif_select_epson(void)
{
	assert(screen > 0);

	g_epsonif_gpio->set(screen, 0);
}

void epsonif_deselect_epson(void)
{
	assert(screen > 0);

	g_epsonif_gpio->set(screen, 1);
}

int epsonif_init(struct pl_gpio *gpio,
		 const struct epson_gpio_config *epson_gpio,
		 int spi_channel, u16 divisor)
{
	g_epsonif_gpio = gpio;
	g_epson_gpio = epson_gpio;
	spi_init(gpio, spi_channel, divisor);

	if (epsonif_init_reset())
		return -1;

	epsonif_assert_reset();
	mdelay(4);
	epsonif_negate_reset();

	screen = -1;
	busy = 0;

	return 0;
}
