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

/** Set to 1 to use the VCOM and hardware info stored in board EEPROM **/
#define CONFIG_HW_INFO_EEPROM   1

/** Set to 1 to use default VCOM calibration settings if HW info EEPROM data
 * cannot be used (either not programmed, or hardware fault, or
 * CONFIG_HW_INFO_EEPROM is not defined).  If set to 0, the system will not be
 * able to work without valid EEPROM data.  */
#define CONFIG_HW_INFO_DEFAULT  1

/** Set to 1 to enable automatic platform detection, which typically uses
 * CONFIG_HW_INFO_EEPROM */
#define CONFIG_PLAT_AUTO        1

/** Set one of the following to 1 to manually select te platform.  This will be
 * used if CONFIG_PLAT_AUTO is not defined, or if no platform can be discovered
 * at runtime.  */
#define CONFIG_PLAT_CUCKOO      0       /**< Cuckoo board (Type4) */
#define CONFIG_PLAT_RAVEN       0       /**< Raven board (Type11) */
#define	CONFIG_PLAT_Z13         0       /**< Hummingbird Z1.3 */
#define CONFIG_PLAT_Z6          0       /**< Hummingbird Z6.x */
#define CONFIG_PLAT_Z7          1       /**< Hummingbird Z7.x */

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
#define CONFIG_DISPLAY_TYPE     "Type19"

/** Use the I2C master bus from the Epson controller (either -524 or -541).
 * Otherwise, use the host I2C master.  */
#define	CONFIG_I2C_ON_EPSON     0

/** Set to 1 to use the power state transition demo rather than the slideshow */
#define CONFIG_DEMO_POWERMODES  0

#endif /* INCLUDE_CONFIG_H */

/** Load the display data from either the display EEPROM or SD card
 * Available options:
 * 		PLWF_EEPROM_SD - Try EEPROM first, then SD card
 *  	PLWF_EEPROM_ONLY - Try EEPROM only
 *  	PLWF_SD_ONLY - Try SD card only
 *  	PLWF_SD_EEPROM - Try SD card first, then EEPROM */
/*#define CONFIG_PLWF_MODE	PLWF_EEPROM_SD*/
