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
 * pl/platform.h
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_PLATFORM_H
#define INCLUDE_PL_PLATFORM_H 1

#include <pl/gpio.h>
#include <pl/i2c.h>
#include <pl/epdpsu.h>
#include <pl/epdc.h>
#include "i2c-eeprom.h"
#include "config.h"

struct pl_hwinfo;

/* Common platform data */

struct pl_system_gpio {
	unsigned sel[4];
	unsigned sw[5];
	unsigned led[4];
	unsigned assert_led;
};

struct platform {
	struct pl_gpio gpio;
	const struct pl_system_gpio *sys_gpio;
	const struct pl_hwinfo *hwinfo;
	struct pl_i2c host_i2c;
	struct pl_i2c disp_i2c;
	struct pl_i2c *i2c;
	struct pl_epdpsu psu;
	struct pl_epdc epdc;
};

#endif /* INCLUDE_PL_PLATFORM_H */
