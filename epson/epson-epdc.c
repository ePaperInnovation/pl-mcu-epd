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
 * epson-epdc.c -- Epson EPDC implementations
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "epson/epson-s1d13524.h"
#include "epson/epson-s1d13541.h"
#include "epson/epson-s1d135xx.h"
#include <epson/epson-epdc.h>
#include <pl/types.h>
#include <pl/epdc.h>
#include <stdlib.h>
#include "assert.h"

#define LOG_TAG "epson-epdc"
#include "utils.h"

#if 0 /* some E-Ink libraries appear to use this convention */
static const struct pl_wfid epson_epdc_wf_table_eink[] = {
	{ wf_refresh,      2 },
	{ wf_delta,        3 },
	{ wf_delta_mono,   1 },
	{ wf_refresh_mono, 3 },
	{ wf_init,         0 },
	{ NULL, 0 }
};
#endif

static int epson_epdc_update(struct pl_epdc *epdc, int wfid)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_update(p, wfid);
}

static int epson_epdc_update_area(struct pl_epdc *epdc, int wfid,
				  const struct pl_area *area)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_update_area(p, wfid, area);
}

static int epson_epdc_wait_update_end(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_wait_update_end(p);
}

static int epson_epdc_set_power(struct pl_epdc *epdc,
				enum pl_epdc_power_state state)
{
	struct s1d135xx *p = epdc->data;

	if (s1d135xx_set_power_state(p, state))
		return -1;

	epdc->power_state = state;

	return 0;
}

int epson_epdc_init(struct pl_epdc *epdc, enum epson_epdc_ref ref,
		    struct s1d135xx *s1d135xx)
{
	int stat;

	assert(epdc != NULL);
	assert(s1d135xx != NULL);
	assert(s1d135xx->data != NULL);

	epdc->update = epson_epdc_update;
	epdc->update_area = epson_epdc_update_area;
	epdc->wait_update_end = epson_epdc_wait_update_end;
	epdc->set_power = epson_epdc_set_power;
	epdc->data = s1d135xx;

	switch (ref) {
	case EPSON_EPDC_S1D13524:
		stat = epson_epdc_init_s1d13524(epdc);
		break;
	case EPSON_EPDC_S1D13541:
		stat = epson_epdc_init_s1d13541(epdc);
		break;
	default:
		abort_msg("Invalid Epson EPDC reference");
	}

	if (stat)
		return -1;

	if (epdc->set_power(epdc, PL_EPDC_RUN))
		return -1;

	if (epdc->set_temp_mode(epdc, PL_EPDC_TEMP_INTERNAL))
		return -1;

	return 0;
}
