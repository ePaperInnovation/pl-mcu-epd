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
 * epdc.c -- EPDC generic implementation
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/epdc.h>
#include <string.h>
#include "assert.h"

const char wf_init[] = WF_INIT;
const char wf_refresh[] = WF_REFRESH;
const char wf_delta[] = WF_DELTA;
const char wf_refresh_mono[] = WF_REFRESH"/"WF_MONO;
const char wf_delta_mono[] = WF_DELTA"/"WF_MONO;

int pl_epdc_get_wfid(struct pl_epdc *p, const char *wf_path)
{
	const struct pl_wfid *wfid;

	assert(p != NULL);
	assert(wf_path != NULL);

	/* Optimised string comparison first */
	for (wfid = p->wf_table; wfid->path != NULL; ++wfid)
		if (wfid->path == wf_path)
			return wfid->id;

	/* Full string compare now */
	for (wfid = p->wf_table; wfid->path != NULL; ++wfid)
		if (!strcmp(wfid->path, wf_path))
			return wfid->id;

	return -1;
}

#if PL_EPDC_STUB
/* ----------------------------------------------------------------------------
 * Stub EPDC implementation
 */

static const struct pl_wfid pl_epdc_stub_wf_table[] = {
	{ wf_refresh,      0 },
	{ wf_delta,        1 },
	{ wf_delta_mono,   2 },
	{ wf_refresh_mono, 3 },
	{ wf_init,         4 },
	{ NULL,           -1 }
};

static int pl_epdc_stub_update(struct pl_epdc *p, int wfid)
{
	return 0;
}

static int pl_epdc_stub_update_area(struct pl_epdc *p, int wfid,
				    const struct pl_area *area)
{
	return 0;
}

static int pl_epdc_stub_wait_update_end(struct pl_epdc *p)
{
	return 0;
}

static int pl_epdc_stub_set_power(struct pl_epdc *p,
				  enum pl_epdc_power_state state)
{
	p->power_state = state;

	return 0;
}

static int pl_epdc_stub_set_temp_mode(struct pl_epdc *p,
				      enum pl_epdc_temp_mode mode)
{
	p->temp_mode = mode;

	return 0;
}

int pl_epdc_stub_init(struct pl_epdc *p)
{
	assert(p != NULL);

	p->update = pl_epdc_stub_update;
	p->update_area = pl_epdc_stub_update_area;
	p->wait_update_end = pl_epdc_stub_wait_update_end;
	p->set_power = pl_epdc_stub_set_power;
	p->set_temp_mode = pl_epdc_stub_set_temp_mode;
	p->wf_table = pl_epdc_stub_wf_table;
	p->xres = 640;
	p->yres = 480;
	p->data = NULL;

	return 0;
}
#endif
