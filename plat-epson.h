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
 * plat-generic.h -- Plastic Logic non-platform specific calls
 *
 * Authors:
 *   Andrew Cox <andrew.cox@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef PLAT_EPSON_H
#define PLAT_EPSON_H 1

struct epson_gpio_config {
	unsigned reset;
	unsigned cs0;
	unsigned hirq;
	unsigned hrdy;
	unsigned hdc;
};

struct platform;
struct pl_hw_info;

extern int plat_epson_init(struct platform *plat,
			   const struct pl_hw_info *pl_hw_info,
			   const struct epson_gpio_config *gpio_epson);

#endif /* PLAT_EPSON_H */
