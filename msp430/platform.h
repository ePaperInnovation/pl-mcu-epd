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
 * platform.h
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <msp430.h>
#include "i2c.h"
#include "i2c-eeprom.h"
#include "config.h"

/* Common platform data */

#define I2C_PSU_EEPROM_ADDR  0x50

struct platform {
	struct i2c_adapter host_i2c;
#if CONFIG_HW_INFO_EEPROM
	struct i2c_eeprom hw_eeprom;
#endif
};

#endif /* PLATFORM_H_ */
