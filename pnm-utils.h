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
 * pnm-utils.h -- Utilities for dealing with PNM format graphics files
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef PNM_UTILS_H
#define PNM_UTILS_H 1

#include <FatFs/ff.h>
#include <stdint.h>

enum {
	PNM_BITMAP,
	PNM_GREYSCALE,
	PNM_UNKNOWN
};

struct pnm_header {
	uint8_t type;
	int width;
	int height;
	int max_gray;
};

#define pnm_read_int(_f) ({			\
		int32_t _value;			\
		pnm_read_int32(_f, &_value);	\
		(int)_value; })

extern int pnm_read_header(FIL *pnm_file, struct pnm_header *hdr);
extern int pnm_read_int32(FIL *pnm_file, int32_t *value);

#endif /* PNM_UTILS_H */
