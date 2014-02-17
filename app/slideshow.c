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
 * app/slideshow.c -- Basic slideshow app
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "app.h"
#include <pl/platform.h>
#include <pl/epdc.h>
#include <pl/epdpsu.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "assert.h"
#include "FatFs/ff.h"
#include "pnm-utils.h"

#define LOG_TAG "slideshow"
#include "utils.h"

/* -- private functions -- */

static int show_image(struct platform *plat, const char *dir,
		      const char *file);

/* -- public entry point -- */

int app_slideshow(struct platform *plat, const char *path)
{
	DIR dir;
	FILINFO f;
	char full_path[MAX_PATH_LEN];
	int dir_open = 0;

	assert(plat != NULL);
	assert(path != NULL);

	while (!app_stop) {
		if (!dir_open) {
			/* (re-)open the directory */
			if (f_opendir(&dir, path) != FR_OK) {
				LOG("Failed to open directory [%s]", path);
				return -1;
			}

			dir_open = 1;
		}

		/* read next entry in the directory */
		if (f_readdir(&dir, &f) != FR_OK) {
			LOG("Failed to read directory entry");
			return -1;
		}

		/* end of the directory reached */
		if (f.fname[0] == '\0') {
			dir_open = 0;
			continue;
		}

		/* skip directories */
		if ((f.fname[0] == '.') || (f.fattrib & AM_DIR))
			continue;

		/* only show PGM files */
		if (!strstr(f.fname, ".PGM"))
			continue;

		if (show_image(plat, path, f.fname)) {
			LOG("Failed to show image");
			return -1;
		}
	}

	return 0;
}

static int show_image(struct platform *plat, const char *dir,
		      const char *file)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	struct pnm_header hdr;
	FIL image_file;
	int wfid;
	int ret = -1;

	wfid = pl_epdc_get_wfid(epdc, wf_refresh);

	if (wfid < 0)
		return -1;

	if (open_image(dir, file, &image_file, &hdr))
		return -1;

	if (epson_loadImageFile(&image_file, 0x0030, 0))
		goto exit_close_file;

	if (epdc->update_temp(epdc))
		goto exit_close_file;

	if (psu->on(psu))
		goto exit_close_file;

	if (epdc->update(epdc, wfid))
		goto exit_close_file;

	if (epdc->wait_update_end(epdc))
		goto exit_close_file;

	if (psu->off(psu))
		goto exit_close_file;

	ret = 0;

exit_close_file:
	f_close(&image_file);

	return ret;
}
