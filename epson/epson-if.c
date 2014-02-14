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

#include "epson-s1d135xx.h"
#include <pl/gpio.h>
#include "plat-epson.h"
#include "types.h"
#include "assert.h"
#include "msp430-spi.h"
#include "msp430-gpio.h"

static struct pl_gpio *g_epsonif_gpio = NULL;
/*static const struct epson_config *g_config = NULL;*/
static const struct s1d135xx_data *g_config = NULL;

int epsonif_init_reset(void)
{
	if (g_config->reset == PL_GPIO_NONE)
		return 0;

	return g_epsonif_gpio->config(g_config->reset,
				      PL_GPIO_OUTPUT | PL_GPIO_INIT_H);
}

int epsonif_read_hrdy(void)
{
	if (g_config->hrdy == PL_GPIO_NONE)
		return 0; /* ToDo: read SPI register if no GPIO */

	return g_epsonif_gpio->get(g_config->hrdy);
}

void epsonif_set_command(void)
{
	if (g_config->hdc != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_config->hdc, 0);
}

void epsonif_set_data(void)
{
	if (g_config->hdc != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_config->hdc, 1);
}

void epsonif_assert_reset(void)
{
	if (g_config->reset != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_config->reset, 0);
}
void epsonif_negate_reset(void)
{
	if (g_config->reset != PL_GPIO_NONE)
		g_epsonif_gpio->set(g_config->reset, 1);
}

void epsonif_select_epson(void)
{
	g_epsonif_gpio->set(g_config->cs0, 0);
}

void epsonif_deselect_epson(void)
{
	g_epsonif_gpio->set(g_config->cs0, 1);
}

int epsonif_init(struct pl_gpio *gpio,
		 const struct s1d135xx_data *config)
{
	g_epsonif_gpio = gpio;
	g_config = config;
	spi_init(gpio, 0, 1);

	if (epsonif_init_reset())
		return -1;

	epsonif_assert_reset();
	mdelay(4);
	epsonif_negate_reset();

	return 0;
}

int epsonif_claim(int spi_channel, screen_t screen_id, screen_t *previous)
{
	return 0;
}

int epsonif_release(int spi_channel, screen_t previous)
{
	return 0;
}

void epsonif_hack(struct pl_gpio *gpio, const struct s1d135xx_data *data)
{
	g_epsonif_gpio = gpio;
	g_config = data;
}
