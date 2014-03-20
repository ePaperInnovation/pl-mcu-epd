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
 * epdpsu.h -- EPD PSU interface abstraction layer
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_EPDPSU_H
#define INCLUDE_PL_EPDPSU_H 1

/**
   @file pl/epdpsu.h

   Abstract interface and generic implementation to the EPD PSU
*/

/** Set to 1 to enable stub  */
#define PL_EPDPSU_STUB 0

struct pl_epdc;

/** Interface */
struct pl_epdpsu {
	/**
	   turn the EPD PSU on
	   @param[in] psu pl_epdpsu instance
	   @return -1 if an error occured, 0 otherwise
	 */
	int (*on)(struct pl_epdpsu *psu);

	/**
	   turn the EPD PSU off
	   @param[in] psu pl_epdpsu instance
	   @return -1 if an error occured, 0 otherwise
	 */
	int (*off)(struct pl_epdpsu *psu);

	int state;            /**< current power state (1=on, 0=off) */
	void *data;           /**< private data for the implementation */
};

/** Generic GPIO-based implementation */
struct pl_epdpsu_gpio {
	struct pl_gpio *gpio; /**< pl_gpio instance to control the GPIOs */
	unsigned hv_en;       /**< GPIO number to turn the power on/off */
	unsigned com_close;   /**< GPIO number to close the COM switch */
	unsigned pok;         /**< GPIO number to read Power OK */
	unsigned timeout_ms;  /**< Maximum time in ms to wait for POK */
	unsigned on_delay_ms; /**< Delay after turning the power on */
	unsigned off_delay_ms;/**< Delay after turning the power off */
};

/**
   Initialise a pl_epdpsu instance with generic GPIO-based implamentation.

   Both the pl_epdpsu and pl_epdpsu_gpio structures need to be managed by the
   caller, so they can be either on the heap or the caller's stack.

   @param[in] psu pl_epdpsu instance
   @param[in] p pl_epdpsu_gpio instance
   @return -1 if an error occured, 0 otherwise
*/
extern int pl_epdpsu_gpio_init(struct pl_epdpsu *psu,
			       struct pl_epdpsu_gpio *p);

/**
   Initialise a pl_epdpsu instance to use the generic epdc->set_epd_power

   @param[in] psu pl_epdpsu instance
   @param[in] epdc pl_epdc instance
   @return -1 if an error occured, 0 otherwise
 */
extern int pl_epdpsu_epdc_init(struct pl_epdpsu *psu, struct pl_epdc *epdc);

#if PL_EPDPSU_STUB
/** Initialise an pl_epdpsu instance with stub implementation.

    This is especially useful with automatic power control or for debugging.

    @param[in] psu pl_epdpsu instance
    @return 0 unless something went really wrong
*/
extern int pl_epdpsu_stub_init(struct pl_epdpsu *psu);
#endif

#endif /* INCLUDE_PL_EPDPSU_H */
