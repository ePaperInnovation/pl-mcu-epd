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

#define LOG_TAG "wflib"
#include "utils.h"

static int pl_wflib_fatfs_rewind(struct pl_wflib *wflib)
{
	FIL *f = wflib->priv;

	return (f_lseek(f, 0) == FR_OK) ? 0 : -1;
}

static size_t pl_wflib_fatfs_read(struct pl_wflib *wflib, uint8_t *data,
				  size_t n)
{
	FIL *f = wflib->priv;
	size_t count;

	if (f_read(f, data, n, &count) != FR_OK)
		return (size_t)-1;

	return count;
}

static void pl_wflib_fatfs_close(struct pl_wflib *wflib)
{
	FIL *f = wflib->priv;

	f_close(f);
}

int pl_wflib_init_fatfs(struct pl_wflib *wflib, FIL *f, const char *path)
{
	if (f_open(f, path, FA_READ) != FR_OK) {
		LOG("Failed to open wflib: %s", path);
		return -1;
	}

	wflib->rewind = pl_wflib_fatfs_rewind;
	wflib->read = pl_wflib_fatfs_read;
	wflib->close = pl_wflib_fatfs_close;
	wflib->size = f_size(f);
	wflib->priv = f;

	LOG("FatFS %s", path);

	return 0;
}
