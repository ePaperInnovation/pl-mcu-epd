/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2014 Plastic Logic Limited

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
 * msp430/plat-gpio.h -- MSP430 optimised GPIO API
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_MSP430_PLAT_GPIO_H
#define INCLUDE_MSP430_PLAT_GPIO_H 1

extern int msp430_gpio_get(unsigned gpio);
extern void msp430_gpio_set(unsigned gpio, int value);

#define pl_gpio_get(_p, _gpio) msp430_gpio_get((_gpio))
#define pl_gpio_set(_p, _gpio, _value) msp430_gpio_set((_gpio), (_value))

#endif /* INCLUDE_MSP430_PLAT_GPIO_H */
