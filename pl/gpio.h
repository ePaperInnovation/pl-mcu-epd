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

/** This value can be used in to indicate that a GPIO is not available */
#define PL_GPIO_NONE ((unsigned)-1)

/** Pin configuration flags */
enum pl_gpio_flags {
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
	/** Configure a GPIO
	    @param[in] gpio GPIO number
	    @param[in] flags flags bitmask using pl_gpio_flags
	    @return -1 if error, 0 otherwise
	 */
	int (*config)(unsigned gpio, uint16_t flags);

	/** Get the value of a GPIO
	    @param[in] gpio GPIO number
	    @return 1 if the GPIO state is high, 0 if low
	 */
	int (*get)(unsigned gpio);

	/** Set the value of a GPIO
	    @param[in] gpio GPIO number
	    @param[in] value value to set the GPIO state
	 */
	void (*set)(unsigned gpio, int value);
};

/** GPIO configuration information */
struct pl_gpio_config {
	unsigned gpio;                  /**< GPIO number */
	uint16_t flags;                 /**< flags bitmask */
};

/** Initialise a list of GPIOs
    @param[in] gpio gpio instance
    @param[in] config array of pl_gpio_config structures
    @param[in] n length of the config array
    @return -1 if error, 0 otherwise
*/
extern int pl_gpio_config_list(struct pl_gpio *gpio,
			       const struct pl_gpio_config *config, size_t n);

/** Log a human-readable version of the flags
    @param[in] flags flags bitmask
*/
extern void pl_gpio_log_flags(uint16_t flags);

/** Check flags are a valid combination
    @param[in] flags flags bitmask
    @return -1 if flags are not valid, 0 otherwise
*/
extern int pl_gpio_check_flags(uint16_t flags);

#endif /* PL_GPIO_H */
