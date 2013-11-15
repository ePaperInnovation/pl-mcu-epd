/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

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
 * S1D13541.h -- Epson S1D13541 controller specific functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef S1D13541_H_
#define S1D13541_H_

// include generic, cross controller definitions
#include "S1D135xx.h"
#include "plwf.h"
#include "types.h"

#define	S1D13541_WF_ADDR 0x00080000L

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

/* -- Initialisation -- */

/* Initialise the controller and leave it in a state ready to do updates */
extern int s1d13541_init_start(screen_t screen, screen_t *previous,
			       struct s1d135xx **controller);
extern int s1d13541_init_prodcode(struct s1d135xx *epson);
extern int s1d13541_init_clock(struct s1d135xx *epson);
extern int s1d13541_init_initcode(struct s1d135xx *epson);
extern int s1d13541_init_pwrstate(struct s1d135xx *epson);
extern int s1d13541_init_keycode(struct s1d135xx *epson);
extern int s1d13541_init_gateclr(struct s1d135xx *epson);
extern int s1d13541_init_end(struct s1d135xx *epson, screen_t previous);


/* -- Display updates -- */

/* Initialise the pixel buffer but do not drive the display */
extern void s1d13541_init_display(struct s1d135xx *epson);

/* Start to update the full display with a given waveform */
extern void s1d13541_update_display(struct s1d135xx *epson, int waveform);

/* Start to update an area of the display with a given waveform */
extern void s1d13541_update_display_area(struct s1d135xx *epson, int waveform,
					 const struct area *area);

/* Wait for the current display update (full or area) to end */
extern void s1d13541_wait_update_end(struct s1d135xx *epson);

/* -- Waveform management -- */

extern int s1d13541_send_waveform(void);

/* -- Temperature management -- */

/* Configure controller for specified temperature mode */
extern void s1d13541_set_temperature_mode(struct s1d135xx *epson,
					  enum s1d135xx_temp_mode temp_mode);

/* Set temperature for manual mode */
extern void s1d13541_set_temperature(struct s1d135xx *epson, int8_t temp);

/* Get last measured temperature */
extern int8_t s1d13541_get_temperature(struct s1d135xx *epson);

/* Measure temperature using specified mode */
extern void s1d13541_measure_temperature(struct s1d135xx *epson,
					 uint8_t *needs_update);


/* -- Power management -- */

/* Set power mode to sleep */
extern int s1d13541_pwrstate_sleep(struct s1d135xx *epson);

/* Set power mode to standby */
extern int s1d13541_pwrstate_standby(struct s1d135xx *epson);

/* Set power mode to run */
extern int s1d13541_pwrstate_run(struct s1d135xx *epson);

#endif /* S1D13541_H_ */
