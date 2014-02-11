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
 * gpio.h -- GPIO interface abstraction layer
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef PL_GPIO_H
#define PL_GPIO_H 1

#include <stdint.h>
#include <stdlib.h>

/**
   @file pl/gpio.h

   Abstract interface to use General Purpose Input / Output signals
*/

/** Pin configuration flags */
enum {
	/* For inputs */
	PL_GPIO_INPUT = 1 << 0,
	PL_GPIO_PU = 1 << 1,
	PL_GPIO_PD = 1 << 2,
	PL_GPIO_INTERRUPT = 1 << 3,
	PL_GPIO_INT_RISE = 1 << 4,
	PL_GPIO_INT_FALL = 1 << 5,

	/* For outputs */
	PL_GPIO_OUTPUT = 1 << 6,
	PL_GPIO_INIT_H = 1 << 7,
	PL_GPIO_INIT_L = 1 << 8,
	PL_GPIO_DRIVE_FULL = 1 << 9,
	PL_GPIO_DRIVE_REDUCED = 1 << 10,

	/* For special features */
	PL_GPIO_SPECIAL = 1 << 11,
};

/** Interface to be populated by concrete implementations */
struct pl_gpio {
	int (*config)(unsigned gpio, uint16_t flags);
	int (*get)(unsigned gpio);
	void (*set)(unsigned gpio, int value);
};

/** GPIO configuration information */
struct pl_gpio_config {
	unsigned gpio;
	uint16_t flags;
};

/** Initialise a list of GPIOs */
extern int pl_gpio_config_list(struct pl_gpio *gpio,
			       const struct pl_gpio_config *config, size_t n);

/** Log a human-readable version of the flags */
extern void pl_gpio_log_flags(uint16_t flags);

/** Check the flags are a valid combination */
extern int pl_gpio_check_flags(uint16_t flags);

#endif /* PL_GPIO_H */
