/*
 * Copyright (C) 2013, 2014 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H 1

/*
 * This file contains global configuration values for the Plastic Logic
 * hardware as well as some various build-time software options.
 */

/** Set to 1 if this CPU is little-endian */
#define CONFIG_LITTLE_ENDIAN          1

/** Set to 1 to use the VCOM and hardware info stored in board EEPROM **/
#define CONFIG_HWINFO_EEPROM          1

/** Set to 1 to use default VCOM calibration settings if HW info EEPROM data
 * cannot be used (either not programmed, or hardware fault, or
 * CONFIG_HWINFO_EEPROM is not defined).  If set to 0, the system will not be
 * able to work without valid EEPROM data.  */
#define CONFIG_HWINFO_DEFAULT         1

/** Set one of the following to 1 to manually select te platform.  This will be
 * used if CONFIG_PLAT_AUTO is not defined, or if no platform can be discovered
 * at runtime.  */
#define CONFIG_PLAT_RAVEN             0 /**< Raven board */
#define CONFIG_PLAT_Z13               0 /**< Hummingbird Z1.3 board */
#define CONFIG_PLAT_Z6                0 /**< Hummingbird Z6.x board */
#define CONFIG_PLAT_Z7                1 /**< Hummingbird Z7.x board */

/** Default I2C master mode used with CONFIG_HWINFO_DEFAULT (see pl/hwinfo.h
 * and plswmanual.pdf for possible values) */
#define CONFIG_DEFAULT_I2C_MODE       I2C_MODE_HOST

/** Each display has a type and some associated data such as a VCOM voltage and
 * waveform library.  This can either be stored in the display EEPROM or on the
 * SD card.  The display type may also be manually specified with
 * CONFIG_DISPLAY_TYPE.
 *
 * Set one of the following values to 1 in order to choose where the data
 * should be read from: */
#define CONFIG_DISP_DATA_EEPROM_ONLY  0 /**< Only use display EEPROM */
#define CONFIG_DISP_DATA_SD_ONLY      1 /**< Only use SD card */
#define CONFIG_DISP_DATA_EEPROM_SD    0 /**< Try EEPROM first, then SD card */
#define CONFIG_DISP_DATA_SD_EEPROM    0 /**< Try SD card first, then EEPROM */

/** Set this to manually specify the display type when it could not be detected
 * at run-time.  This is especially useful for displays without an EEPROM such
 * as Type19.  */
#define CONFIG_DISPLAY_TYPE           "Type19"

/** Set to 1 to use the power state transition demo rather than the slideshow */
#define CONFIG_DEMO_POWERMODES        0

#define CONFIG_DEMO_PATTERN           0 /** NOT INTENDED FOR USE ON Type 19 displays  */
#define CONFIG_DEMO_PATTERN_SIZE      16

#endif /* INCLUDE_CONFIG_H */
