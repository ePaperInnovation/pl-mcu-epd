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
 * slideshow.h -- simple slideshow functionality
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef SLIDESHOW_H_
#define SLIDESHOW_H_

#include "types.h"
#include <stdint.h>

/** Slideshow item with regions, waveform and timing information */
struct slideshow_item {
	char file[32];          /**< path to the image file to open */
	struct area area;       /**< area coordinates on the display */
	int left_in;            /**< left coordinate to start reading from */
	int top_in;             /**< top coordinate to start reading from */
};

/** Slideshow callback function, called on each file found in a directory */
typedef int (*slideshow_cb_t)(const char *path, void *arg);

/** Iterate over a directory invoking the callback to display each image */
extern int slideshow_run(const char *path, slideshow_cb_t cb, void *arg);

/** Load the contents of an image file into the Epson controller */
extern int slideshow_load_image(const char *image, uint16_t mode, int pack);

/** Load an area of the contents of an image file into the Epson controller
    ToDo: deprecate slideshow_load_image */
extern int slideshow_load_image_area(const struct slideshow_item *item,
				     const char *dir, uint16_t mode, int pack);

/** Parse a CSV line of text into a slideshow item structure */
extern int slideshow_parse_item(const char *line, struct slideshow_item *item);

#endif /* SLIDESHOW_H_ */
