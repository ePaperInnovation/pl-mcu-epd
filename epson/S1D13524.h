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
/*
 * S1D13524.h -- Epson S1D13524 specific controller functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * Note: There are many more registers than documented here. They have the correct
 * default value configured by the controller configuration file.
 */

#ifndef S1D13524_H_
#define S1D13524_H_

// include generic, cross controller definitions
#include "S1D135xx.h"

// Waveform read control register
#define WAVEFORM_RD_CONF_REG				0x0260 // Waveform read configuration register

// Display Engine: Display Timing Configuration
#define DISPLAY_FRAME_DATA_LG_REG			0x0300 // Frame data length register
#define DISPLAY_PNL_BP_DATA_LG_REG			0x0306 // Panel Backplane line data length register

// Display Engine: Driver Configuration
#define SRC_GATE_CONF_REG					0x030E // source / gate driver configuration register

// waveform register
#define WAVE_ADD_REG0						0x0390 // Waveform header address register 0
#define WAVE_ADD_REG1						0x0392 // Waveform header address register 1

// Temperature sensing control
#define	TEMP_AUTO_RETRIEVE_REG				0x0320	// Enable/Disable Auto Temperature sensing
#define	TEMP_VALUE_REG						0x0322	// Waveform selection temperature (signed)

int s1d13524_init(screen_t screen, struct s1d135xx **epson);
int s1d13524_update_display(struct s1d135xx *epson, int waveform);

int s1d13524_set_temperature_mode(struct s1d135xx *epson, short temp_mode);
int s1d13524_set_temperature(struct s1d135xx *epson, s8 temp);
int s1d13524_get_temperature(struct s1d135xx *epson, s8 *temp);
int s1d13524_measure_temperature(struct s1d135xx *epson);


#endif /* S1D13524_H_ */
