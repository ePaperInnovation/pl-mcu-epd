/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2014 Plastic Logic Limited

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
 * wflib.h -- Waveform library management
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_PL_WFLIB_H
#define INCLUDE_PL_WFLIB_H 1

#include <FatFs/ff.h>
#include <stdint.h>
#include <stdlib.h>

struct pl_wflib {
	int (*rewind)(struct pl_wflib *p);
	size_t (*read)(struct pl_wflib *p, uint8_t *data, size_t n);
	void (*close)(struct pl_wflib *p);
	uint32_t size;
	void *priv;
};

/** Initialise a wflib interface for a FatFS file */
extern int pl_wflib_init_fatfs(struct pl_wflib *wflib, FIL *f,
			       const char *path);

#endif /* INCLUDE_PL_WFLIB_H */
