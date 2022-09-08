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
 * app/slideshow.c -- Basic slideshow app
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "app.h"
#include <pl/platform.h>
#include <pl/epdc.h>
#include <pl/epdpsu.h>
#include <stdio.h>
#include <string.h>
#include "assert.h"

#define LOG_TAG "slideshow"
#include "utils.h"

/* -- private functions -- */
/*
static int show_image(struct pl_platform *plat);

/* -- public entry point -- */

int app_slideshow(struct pl_platform *plat, const char *path)
{

 //   int dir_open = 0;

    assert(plat != NULL);
    assert(path != NULL);

    LOG("Running slideshow");
    show_image(plat);

 //   int updates = 2;

    while (!app_stop)
    {

            continue;

        // only show PGM files
      //  if (!strstr(f.fname, ".PGM"))
        //    continue;

        if (show_image(plat))
        {
            LOG("Failed to show image");
            return -1;
        }
//		updates--;
//		if (updates == 0)
//		    app_stop = 1;
    }
    return 0;
}

static int show_image(struct pl_platform *plat)
{
    struct pl_epdc *epdc = &plat->epdc;
    struct pl_epdpsu *psu = &plat->psu;
    char path[MAX_PATH_LEN];
    int wfid;

    wfid = 2;

//	if (wfid < 0)
//		return -1;

 //   join_path(path, sizeof(path), dir, file);

    epdc->wait_update_end(epdc);

    epdc->load_image(epdc, path, NULL, 0, 0);

    epdc->update_temp(epdc);

    //if (psu->on(psu))
//		return -1;

    epdc->update(epdc, 7, UPDATE_FULL, NULL);

    epdc->wait_update_end(epdc);

//	if (psu->off(psu))
//		return -1;

    return 0;
}
