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

#include "pl/hwinfo.h"
/*
 * This file contains global configuration values for the Plastic Logic
 * hardware as well as some various build-time software options.
 */

/** Set to 1 if this CPU is little-endian */
enum endianess {
	CONFIG_BIG_ENDIAN = 0,
	CONFIG_LITTLE_ENDIAN
};
/** Set to 1 to use the VCOM and hardware info stored in board EEPROM */
#define CONFIG_HWINFO_EEPROM          1

/** Set to 1 to use default VCOM calibration settings if HW info EEPROM data
 * cannot be used (either not programmed, or hardware fault, or
 * CONFIG_HWINFO_EEPROM is not defined).  If set to 0, the system will not be
 * able to work without valid EEPROM data.  */
#define CONFIG_HWINFO_DEFAULT         1

/** Set one of the following to 1 to manually select the platform.
 * This will be used no platform can be discovered at runtime.  */
enum config_platform_board {
	CONFIG_PLAT_RAVEN, /**< Raven board */
	CONFIG_PLAT_Z6, /**< Hummingbird Z6.x board */
	CONFIG_PLAT_Z7  /**< Hummingbird Z7.x board */
};

/** Each display has a type and some associated data such as a VCOM voltage and
 * waveform library.  This can either be stored in the display EEPROM or on the
 * SD card.  The display type may also be manually specified with
 * CONFIG_DISPLAY_TYPE.
 *
 * Set one of the following values to 1 in order to choose where the data
 * should be read from: */
enum config_data_source {
	CONFIG_DISP_DATA_EEPROM_ONLY,  /**< Only use display EEPROM */
	CONFIG_DISP_DATA_SD_ONLY,       /**< Only use SD card */
	CONFIG_DISP_DATA_EEPROM_SD,    /**< Try EEPROM first, then SD card */
	CONFIG_DISP_DATA_SD_EEPROM    /**< Try SD card first, then EEPROM */
};

/** Set to 1 to use the power state transition demo rather than the slideshow */
#define CONFIG_DEMO_POWERMODES        0

/** Set to 1 to use the pattern demo rather than the slideshow */
#define CONFIG_DEMO_PATTERN           0  /** Not intended for Type19 displays  */
#define CONFIG_DEMO_PATTERN_SIZE      16 /** Size of checker-board */

/** Set to 1 to have stdout, stderr sent to serial port */
#define CONFIG_UART_PRINTF		0

struct config {
	enum endianess endianess; // most likely always little endian
	enum i2c_mode_id i2c_mode;
	enum config_data_source data_source; //SD-Card or EEPROM or both
	enum config_platform_board board; //HBZ6, 7, 8, 9 or Raven
	char config_display_type[16];
	int scrambling;
	int waveform_version;
};

extern struct config global_config;

int read_config(char* configfile, struct config* config);

#endif /* INCLUDE_CONFIG_H */
