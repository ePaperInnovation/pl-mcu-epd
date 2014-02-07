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
 * S1D135xx.c -- Controller common functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "types.h"
#include "assert.h"
#include "S1D135xx.h"
#include "epson-if.h"
#include <string.h>

static const char _wf_init[] = WF_INIT;
static const char _wf_refresh[] = WF_REFRESH;
static const char _wf_delta[] = WF_DELTA;
static const char _wf_refresh_mono[] = WF_REFRESH"/"WF_MONO;
static const char _wf_delta_mono[] = WF_DELTA"/"WF_MONO;

const char * const wf_init = _wf_init;
const char * const wf_refresh = _wf_refresh;
const char * const wf_delta = _wf_delta;
const char * const wf_refresh_mono = _wf_refresh_mono;
const char * const wf_delta_mono = _wf_delta_mono;

struct wfid {
	const char *path;
	int id;
};

static const struct wfid wfid_s1d13541[] = {
	{ _wf_refresh,      1 },
	{ _wf_delta,        3 },
	{ _wf_delta_mono,   2 },
	{ _wf_refresh_mono, 4 },
	{ _wf_init,         0 },
	{ NULL, 0 }
};

static const struct wfid wfid_s1d13524[] = {
	{ _wf_refresh,      2 },
	{ _wf_delta,        3 },
	{ _wf_delta_mono,   4 },
	{ _wf_refresh_mono, 1 },
	{ _wf_init,         0 },
	{ NULL, 0 }
};

#if 0 /* some E-Ink libraries appear to use this convention */
static const struct wfid wfid_eink[] = {
	{ _wf_refresh,      2 },
	{ _wf_delta,        3 },
	{ _wf_delta_mono,   1 },
	{ _wf_refresh_mono, 3 },
	{ _wf_init,         0 },
	{ NULL, 0 }
};
#endif

const static struct wfid *wfid_table;

int s1d135xx_set_wfid_table(int epdc)
{
	switch (epdc) {
	case EPDC_S1D13524:
		wfid_table = wfid_s1d13524;
		break;
	case EPDC_S1D13541:
		wfid_table = wfid_s1d13541;
		break;
	default:
		/* Default to use '541 table */
		wfid_table = wfid_s1d13541;
		break;
	}
	return 0;
}

int s1d135xx_get_wfid(const char *wf_path)
{
	const struct wfid *wfid;

	assert(wf_path != NULL);

	/* Optimised string comparison first */
	for (wfid = wfid_table; wfid->path != NULL; ++wfid)
		if (wfid->path == wf_path)
			return wfid->id;

	/* Full string compare now */
	for (wfid = wfid_table; wfid->path != NULL; ++wfid)
		if (!strcmp(wfid->path, wf_path))
			return wfid->id;

	return -1;
}

int s1d135xx_select(struct s1d135xx *epson, screen_t *previous)
{
	assert(epson != NULL);
	assert(previous != NULL);

	return epsonif_claim(0, epson->screen, previous);
}

int s1d135xx_deselect(struct s1d135xx *epson, screen_t previous)
{
	assert(epson);

	return epsonif_release(0, previous);
}
