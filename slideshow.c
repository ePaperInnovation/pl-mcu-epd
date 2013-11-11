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
 * slideshow.c -- simple slideshow functionality
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "assert.h"
#include "epson/epson-utils.h"
#include "FatFs/ff.h"
#include "pnm-utils.h"
#include "slideshow.h"
#include "utils.h"

#define LOG_TAG "slides"

/* FatFS only supports 8.3 filenames, and we work from the current directory so
   paths should be short... */
#define MAX_PATH_LEN 64

int slideshow_run(const char *path, slideshow_cb_t callback, void *arg)
{
	DIR dir;
	FILINFO fno;
	char full_path[MAX_PATH_LEN];
	int result = 0;

	assert(callback != NULL);
	assert(path != NULL);

	if (f_opendir(&dir, path) != FR_OK)
		return -1;

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

	} while (result >= 0);

	return result;
}

static int load_image_area(const char *image, uint16_t mode, int pack,
			   const struct area *area, int left, int top)
{
	FIL image_file;
	struct pnm_header hdr;
	int ret;

	if (f_open(&image_file, image, FA_READ) != FR_OK) {
		LOG("Failed to open image file");
		return -1;
	}

	ret = pnm_read_header(&image_file, &hdr);

	if (ret < 0) {
		LOG("Failed to parse PGM header");
		goto err_close_file;
	}

#if 0
	if (area != NULL)
		LOG("area: %p (%d, %d) %dx%d",
		    area, area->left, area->top, area->width, area->height);
#endif

	if (area == NULL)
		ret = epson_loadImageFile(&image_file, mode, pack);
	else
		ret = epson_loadImageFileArea(&image_file, mode, pack,
					      area, left, top, hdr.width);

err_close_file:
	f_close(&image_file);

	return ret;
}

int slideshow_load_image(const char *image, uint16_t mode, int pack)
{
	assert(image != NULL);

	return load_image_area(image, mode, pack, NULL, 0, 0);
}

int slideshow_load_image_area(const struct slideshow_item *item,
			      const char *dir, uint16_t mode, int pack)
{
	char path[MAX_PATH_LEN];

	assert(item != NULL);
	assert(dir != NULL);

	if (snprintf(path, MAX_PATH_LEN, "%s/%s", dir, item->file) >=
	    MAX_PATH_LEN) {
		LOG("File path is too long, max=%d", MAX_PATH_LEN);
		return -1;
	}

#if 0
	LOG("slideshow_load_image_area [%s] (%d, %d) %dx%d",
	    path, item->area.left,
	    item->area.top, item->area.width, item->area.height);
#endif

	return load_image_area(path, mode, pack, &item->area,
			       item->left_in, item->top_in);
}

int slideshow_parse_item(const char *line, struct slideshow_item *item)
{
	int *coords[] = {
		&item->left_in, &item->top_in, &item->area.left,
		&item->area.top, &item->area.width, &item->area.height,
		NULL
	};
	static const char sep[] = ", ";
	const char *opt;
	int len;

	assert(line != NULL);
	assert(item != NULL);

	opt = line;
	len = parser_read_str(opt, sep, item->file, sizeof(item->file));

	if (len <= 0)
		goto exit_now;

	opt += len;
	len = parser_read_int_list(opt, sep, coords);

	if (len <= 0)
		goto exit_now;

#if 0
	LOG("%s (%d, %d) -> (%d, %d) %dx%d",
	    item->file, item->left_in, item->top_in,
	    item->area.left, item->area.top,
	    item->area.width, item->area.height);
#endif

	return 0;

exit_now:
	if (!len)
		LOG("Not enough arguments");

	return -1;
}
