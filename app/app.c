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
 * app/app.c -- Application
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/platform.h>

#define LOG_TAG "app"
#include "utils.h"

/* Interim implementation */
#include <epson/epson-s1d135xx.h>
#include <epson/epson-utils.h>
#include <epson/S1D135xx.h>
#include <epson/S1D13541.h>

static const char SLIDES_PATH[] = "img/slides.txt";

int app_stop = 0;

int app_demo(struct platform *plat)
{
	int stat;

	if (app_clear(plat))
		return -1;

	if (is_file_present(SLIDES_PATH))
		stat = app_sequencer(plat, SLIDES_PATH);
	else
		stat = app_slideshow(plat, "img");

	return stat;
}

int app_clear(struct platform *plat)
{
	struct s1d135xx *s1d135xx = plat->epdc.data;
	struct _s1d135xx *epson = s1d135xx->epson;
	struct pl_epdpsu *psu = &plat->psu;
	struct pl_epdc *epdc = &plat->epdc;

	/* Interim implementation using direct Epson functions */
	epson_fill_buffer(0x0030, false, epson->yres, epson->xres, 0xff);
	s1d13541_init_display(epson);

#if 1
	{
		uint8_t needs_update;

		s1d13541_measure_temperature(epson, &needs_update);

		if (needs_update) {
			LOG("needs update...");

			if (s1d13541_send_waveform())
				return -1;
		}
	}
#endif

	if (psu->on(psu))
		return -1;

	if (epdc->update(epdc, pl_epdc_get_wfid(epdc, wf_init)))
		return -1;

	if (epdc->wait_update_end(epdc))
		return -1;

	return psu->off(psu);
}
