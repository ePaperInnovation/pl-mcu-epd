/*
 * Copyright (C) 2013 Plastic Logic Limited
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

/* Select one of the platforms below for 10.7" (Type4 or Type11) displays and
 * the S1D13524 Epson controller. */
#define PLAT_CUCKOO             0       /**< Cuckoo board (Type4) */
#define PLAT_RAVEN              0       /**< Raven board (Type11) */

/* These platforms can drive a range of small displays as defined below using
 * the S1D13541 Epson controller. */
#define	PLAT_Z13                0      /**< Hummingbird Z1.3 */
#define PLAT_Z6                 1      /**< Hummingbird Z6.x */
#define PLAT_Z7                 0      /**< Hummingbird Z7.x */

/* Select one of the following small display types when using the S1D13541
 * controller.  These strings effectively map to the directory path on the SD
 * card to collect the correct data. */
#define	DISPLAY_TYPE16          "0:/Type-16" /**< 4.7" 320x240 */
#define	DISPLAY_TYPE18          "0:/Type-18" /**< 4.0" 400x240 */
#define	DISPLAY_TYPE19          "0:/Type-19" /**< 4.9" 720x120 */
#define CONFIG_DISPLAY_TYPE     DISPLAY_TYPE18

/** Use the I2C master bus from the Epson controller (either -524 or -541) */
#define	CONFIG_I2C_ON_EPSON     0

/** Load the waveform data from the SD card when set to 1, or from the display
 * EEPROM when set to 0 (standard behaviour) */
#define CONFIG_WF_ON_SD_CARD	0

#endif /* INCLUDE_CONFIG_H */
