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
 * display.h -- Display related data
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

struct display {
	int16 xres;				// x resolution
	int16 yres;				// y resolution
	u8    gate_init;		// gate initialisation order (see Epson data sheet)
	u8	  drivers;			// to allow correct PMIC powerup sequence
	u8	  rotation;			// rotation, where 0,0 is
	char  type[MAX_TYPE];	// name of the display
};

/*
 * requires:
 * controller-code location
 * waveform location
 */

#endif /* DISPLAY_H_ */
