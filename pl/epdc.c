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
#include <pl/epdpsu.h>
#if PL_EPDC_STUB
#include <pl/types.h>
#endif
#include <string.h>
#include "assert.h"

#define LOG_TAG "epdc"
#include "utils.h"

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

int pl_epdc_single_update(struct pl_epdc *epdc, struct pl_epdpsu *psu,
			  int wfid, const struct pl_area *area)
{
	if (epdc->update_temp(epdc))
		return -1;

	if (psu->on(psu))
		return -1;

	if (epdc->update(epdc, wfid, area))
		return -1;

	if (epdc->wait_update_end(epdc))
		return -1;

	return psu->off(psu);
}

#if PL_EPDC_STUB
/* ----------------------------------------------------------------------------
 * Stub EPDC implementation
 */

/* Set to 1 to enable verbose log messages */
#define STUB_VERBOSE 1

#if STUB_VERBOSE
#define STUB_LOG(msg, ...) LOG("stub "msg, ##__VA_ARGS__)
#else
#define STUB_LOG(msg, ...)
#endif

static const struct pl_wfid stub_wf_table[] = {
	{ wf_refresh,      0 },
	{ wf_delta,        1 },
	{ wf_delta_mono,   2 },
	{ wf_refresh_mono, 3 },
	{ wf_init,         4 },
	{ NULL,           -1 }
};

static int stub_init(struct pl_epdc *p, uint8_t grey)
{
	STUB_LOG("init grey=0x%02X", grey);

	mdelay(1000);

	return 0;
}

static int stub_load_wflib(struct pl_epdc *p)
{
	STUB_LOG("load_wflib");

	return 0;
}

static int stub_update(struct pl_epdc *p, int wfid, const struct pl_area *area)
{
#if STUB_VERBOSE
	if(area != NULL)
		STUB_LOG("update wfid=%d, start=(%d, %d), dim=%dx%d",
		    wfid, area->left, area->top, area->width, area->height);
	else
		STUB_LOG("update wfid=%d", wfid);
#endif

	return 0;
}

static int stub_wait_update_end(struct pl_epdc *p)
{
	STUB_LOG("wait_update_end");

	mdelay(500);

	return 0;
}

static int stub_set_power(struct pl_epdc *p, enum pl_epdc_power_state state)
{
	STUB_LOG("set_power state=%d", state);

	p->power_state = state;

	return 0;
}

static int stub_set_temp_mode(struct pl_epdc *p, enum pl_epdc_temp_mode mode)
{
	STUB_LOG("set_temp_mode mode=%d", mode);

	p->temp_mode = mode;

	return 0;
}

static int stub_update_temp(struct pl_epdc *p)
{
	STUB_LOG("update_temp");

	return 0;
}

static int stub_fill(struct pl_epdc *p, const struct pl_area *area, uint8_t g)
{
#if STUB_VERBOSE
	if (area != NULL)
		STUB_LOG("fill grey=0x%02X, start=(%d, %d), dim=%dx%d",
			 g, area->left, area->top, area->width, area->height);
	else
		STUB_LOG("fill grey=%0x%02X");
#endif

	return 0;
}

static int stub_load_image(struct pl_epdc *p, const char *path,
			   const struct pl_area *area, int left, int top)
{
#if STUB_VERBOSE
	if (area != NULL)
		STUB_LOG("fill_image path=%s, start=(%d, %d), dim=%dx%d, "
			 "left=%d, top=%d", path, area->left, area->top,
			 area->width, area->height, left, top);
	else
		STUB_LOG("fill_image path=%s, left=%d, top=%d",
			 path, left, top);
#endif

	return 0;
}

int pl_epdc_stub_init(struct pl_epdc *p)
{
	STUB_LOG("stub init");

	assert(p != NULL);

	p->init = stub_init;
	p->load_wflib = stub_load_wflib;
	p->update = stub_update;
	p->wait_update_end = stub_wait_update_end;
	p->set_power = stub_set_power;
	p->set_temp_mode = stub_set_temp_mode;
	p->update_temp = stub_update_temp;
	p->fill = stub_fill;
	p->load_image = stub_load_image;
	p->wf_table = stub_wf_table;
	p->xres = 640;
	p->yres = 480;
	p->data = NULL;

	LOG("Stub ready");

	return 0;
}
#endif
