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
 * app/pattern.c -- Pattern generator app
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *   Andrew Cox <andrew.cox@plasticlogic.com>
 *
 */

#include "app.h"
#include <pl/platform.h>
#include <pl/epdc.h>
#include <pl/epdpsu.h>
#include <stdio.h>
#include <string.h>
#include "assert.h"

#define LOG_TAG "pattern-demo"
#include "utils.h"

int app_pattern(struct pl_platform *plat)
{
	struct pl_epdc *epdc = &plat->epdc;
	struct pl_epdpsu *psu = &plat->psu;
	int wfid; /* = pl_epdc_get_wfid(epdc, wf_refresh);*/

	wfid = pl_epdc_get_wfid(epdc, wf_refresh);

	if (wfid < 0)
		return -1;

	if (epdc->pattern_check(epdc, CONFIG_DEMO_PATTERN_SIZE))
		return -1;

	if (epdc->update_temp(epdc))
		return -1;

	if (psu->on(psu))
		return -1;

	if (epdc->update(epdc, wfid, NULL))
		return -1;

	if (epdc->wait_update_end(epdc))
		return -1;

	if (psu->off(psu))
		return -1;

	return 0;
}
