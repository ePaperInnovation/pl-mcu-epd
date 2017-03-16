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
#include "app.h"

#define LOG_TAG "app"
#include "utils.h"

static const char SLIDES_PATH[] = "img/slides.txt";

int app_stop = 0;

int app_demo(struct pl_platform *plat)
{
	int stat;

	if (app_clear(plat))
		return -1;

	if (CONFIG_DEMO_POWERMODES)
		stat = app_power(plat, "img");
	else if (CONFIG_DEMO_PATTERN)
		stat = app_pattern(plat);
	else if (is_file_present(SLIDES_PATH))
		stat = app_sequencer(plat, SLIDES_PATH);
	else
		stat = app_slideshow(plat, "img");

	return stat;
}

#include <pl/endian.h>

int app_clear(struct pl_platform *plat)
{
	struct pl_epdpsu *psu = &plat->psu;
	struct pl_epdc *epdc = &plat->epdc;
	LOG("Clearing the screen");

	if (psu->on(psu))
		return -1;

	if (epdc->fill(epdc, NULL, PL_WHITE))
		return -1;

	if (epdc->clear_init(epdc))
		return -1;


	if (epdc->update(epdc, 0, UPDATE_FULL, NULL))
		return -1;

	if (epdc->wait_update_end(epdc))
		return -1;

	return psu->off(psu);
}
