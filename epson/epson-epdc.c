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

#include <pl/types.h>
#include <epson/epson-epdc.h>
#include <stdlib.h>
#include "assert.h"

#define LOG_TAG "epson-epdc"
#include "utils.h"

#if EPSON_INTERIM
#include "S1D135xx.h"
static struct s1d135xx g_epson;
#endif

static const struct pl_wfid epson_epdc_wf_table_s1d13524[] = {
	{ wf_refresh,      2 },
	{ wf_delta,        3 },
	{ wf_delta_mono,   4 },
	{ wf_refresh_mono, 1 },
	{ wf_init,         0 },
	{ NULL,           -1 }
};

static const struct pl_wfid epson_epdc_wf_table_s1d13541[] = {
	{ wf_refresh,      1 },
	{ wf_delta,        3 },
	{ wf_delta_mono,   2 },
	{ wf_refresh_mono, 4 },
	{ wf_init,         0 },
	{ NULL,           -1 }
};

static int epson_epdc_update(struct pl_epdc *p, int wfid)
{
	struct s1d135xx *epson = p->data;

	s1d13541_update_display(epson, wfid);

	return 0;
}

static int epson_epdc_update_area(struct pl_epdc *p, int wfid,
				  const struct pl_area *area)
{
	struct s1d135xx *epson = p->data;

	s1d13541_update_display_area(epson, wfid, (struct area *)area);

	return 0;
}

static int epson_epdc_wait_idle(struct pl_epdc *p)
{
	struct s1d135xx *epson = p->data;

	s1d13541_wait_update_end(epson);

	return 0;
}

static int epson_epdc_set_power(struct plepdc *p,
				enum pl_epdc_power_state state)
{
	LOG("set_power (%d)", state);

	return -1;
}

int epson_epdc_init(struct pl_epdc *epdc, enum epson_epdc_ref ref)
{
	LOG("init %d", ref);

	switch (ref) {
	case EPSON_EPDC_S1D13524:
		epdc->wf_table = epson_epdc_wf_table_s1d13524;
		break;
	case EPSON_EPDC_S1D13541:
		epdc->wf_table = epson_epdc_wf_table_s1d13541;
		break;
	default:
		abort_msg("Invalid Epson EPDC reference");
	}

	epdc->update = epson_epdc_update;
	epdc->update_area = epson_epdc_update_area;
	epdc->wait_idle = epson_epdc_wait_idle;
	epdc->set_power = epson_epdc_set_power;

#if EPSON_INTERIM
	epdc->data = &g_epson;

	if (s1d13541_early_init(&g_epson))
		return -1;

	if (s1d13541_init(&g_epson))
		return -1;

	s1d13541_set_temperature_mode(&g_epson, TEMP_MODE_INTERNAL);

	epdc->xres = g_epson.xres;
	epdc->yres = g_epson.yres;
#endif

#if 0
	epson_fill_buffer(0x0030, false, g_epson.yres, g_epson.xres, 0xff);
	s1d13541_init_display(&g_epson);
#endif

	LOG("init OK");

	return 0;
}
