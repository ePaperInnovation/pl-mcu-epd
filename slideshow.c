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
 * slideshow.c -- simple slideshow functionality
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "assert.h"
#include "epson/epson-utils.h"
#include "FatFs/ff.h"
#include "pnm-utils.h"

static DIR dir;
static FILINFO fno;
// we only support 8.3 filenames, and we work from the current directory
// so paths should be short....
static char full_path[50];

/* Iterate over a directory invoking the callback to display each image
 */
int slideshow_run(char *path, int (*callback)(char *path, void *arg), void *arg)
{
	int result = 0;

	assert(callback);
	assert(path);

	if (f_opendir(&dir, path) != FR_OK)
		return -EIO;

	do {
		if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0)
			break;

		/* skip directories */
		if ((fno.fname[0] == '.') || (fno.fattrib & AM_DIR))
			continue;

		/* .. and files without the PGM extension */
		if (!strstr(fno.fname, ".PGM"))
			continue;

		sprintf(full_path, "%s/%s", path, fno.fname);

		result = callback(full_path, arg);

	} while ( result >= 0);

	return result;
}


int slideshow_load_image(const char *image, short mode, int pack)
{
	FIL image_file;
	struct pnm_header hdr;
	int ret;

	assert(image);

	if (f_open(&image_file, image, FA_READ) != FR_OK)
		return -ENOENT;

	if ((ret = pnm_read_header(&image_file, &hdr)) < 0)
		goto err_format;

	// Load 1Bpp image into controller, optionally request it be packed into
	// 4bpp before transfer to the controller
	ret = epson_loadImageFile(&image_file, mode, pack);

err_format:
	f_close(&image_file);

	return ret;
}
