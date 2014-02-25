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
 * wflib.c -- Waveform library management
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/wflib.h>
#include <i2c-eeprom.h>

#define LOG_TAG "wflib"
#include "utils.h"

/* --- FatFS -- */

#define DATA_BUFFER_LENGTH 256

static int pl_wflib_fatfs_xfer(struct pl_wflib *wflib, pl_wflib_wr_t wr,
			       void *ctx)
{
	FIL *f = wflib->priv;
	size_t left = wflib->size;

	if (f_lseek(f, 0) != FR_OK)
		return -1;

	while (left) {
		uint8_t data[DATA_BUFFER_LENGTH];
		const size_t n = min(left, sizeof(data));
		size_t count;

		if ((f_read(f, data, n, &count) != FR_OK) || (count != n)) {
			LOG("Failed to read from file");
			return -1;
		}

		if (wr(wflib, ctx, data, n))
			return -1;

		left -= n;
	}

	return 0;
}

int pl_wflib_init_fatfs(struct pl_wflib *wflib, FIL *f, const char *path)
{
	if (f_open(f, path, FA_READ) != FR_OK) {
		LOG("Failed to open wflib: %s", path);
		return -1;
	}

	wflib->xfer = pl_wflib_fatfs_xfer;
	wflib->size = f_size(f);
	wflib->priv = f;

	LOG("FatFS %s", path);

	return 0;
}
