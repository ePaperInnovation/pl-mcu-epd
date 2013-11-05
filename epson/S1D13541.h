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
 * S1D13541.h -- Epson S1D13541 controller specific functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef S1D13541_H_
#define S1D13541_H_

// include generic, cross controller definitions
#include "S1D135xx.h"

// Display Engine: Control/Trigger register
#define DISPLAY_UPD_BUFF_CONF_REG			0x0330 // Panel Update Buffer Configuration register
#define DISPLAY_UPD_BUFF_PXL_VAL_REG		0x0332 // Panel Update Buffer Pixel set value register
#define DISPLAY_CTRL_TRIG_REG				0x0334 // display engine control / trigger register

#define REG_FRAME_DATA_LENGTH				0x0400 // Frame data length register
#define REG_LINE_DATA_LENGTH				0x0406 // Panel Backplane line data length register
#define REG_PROTECTION_KEY_1				0x042C // Protection Key Code 0
#define REG_PROTECTION_KEY_2				0x042E // Protection Key Code 1

#define	REG_WAVEFORM_DECODER_BYPASS			0x0420

#define	TEMP_SENSOR_EXTERNAL		0x0040	/* 1 to select external temp sensor */
#define	TEMP_SENSOR_CONTROL			0x4000	/* 1 for software control, 0 for hardware control */

#define	AUTO_TEMP_JUDGE_ENABLE		0x0004	/* enable automatic temperature/waveform review */

int s1d13541_init(screen_t screen, struct s1d135xx **controller);
int s1d13541_init_display(struct s1d135xx *epson);
int s1d13541_update_display(struct s1d135xx *epson, int vaveform);

int s1d13541_send_waveform(void);

int s1d13541_set_temperature_mode(struct s1d135xx *epson, short temp_mode);
int s1d13541_set_temperature(struct s1d135xx *epson, s8 temp);
int s1d13541_get_temperature(struct s1d135xx *epson, s8 *temp);
int s1d13541_measure_temperature(struct s1d135xx *epson, u8 *needs_update);

#endif /* S1D13541_H_ */