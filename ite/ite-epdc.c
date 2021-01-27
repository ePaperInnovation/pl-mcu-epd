/*
 *  Plastic Logic EPD project on MSP430

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
 *
 * ite-epdc.c
 *
 *  Created on: 04.01.2021
 *      Author: oliver.lenz
 */

#include <ite/ite-epdc.h>

#define LOG_TAG "ite-epdc"
#include "utils.h"

I80IT8951DevInfo *pBuf;

static int ite_epdc_clear_init(struct pl_epdc *epdc)
{
    struct it8951 *p = epdc->data;

    return it8951_clear_init(p);
}

static int ite_epdc_update(struct pl_epdc *epdc, int wfid,
                           enum pl_update_mode mode, const struct pl_area *area)
{
    struct it8951 *p = epdc->data;

    return it8951_update(p, wfid, mode, area);
}

static int ite_epdc_wait_update_end(struct pl_epdc *epdc)
{
    struct it8951 *p = epdc->data;

    return it8951_wait_update_end(p);
}

static int ite_epdc_set_power(struct pl_epdc *epdc,
                              enum pl_epdc_power_state state)
{
    struct it8951 *p = epdc->data;

    if (it8951_set_power_state(p, state))
        return -1;

    epdc->power_state = state;

    return 0;
}

static int ite_epdc_set_epd_power(struct pl_epdc *epdc, int on)
{
    struct it8951 *p = epdc->data;

    return it8951_set_epd_power(p, on);
}

static int ite_epdc_update_temp(struct pl_epdc *p)
{
    struct it8951 *it8951 = p->data;

    it8951_update_Temp(it8951, 0, 23);
    return 0;
}

static int ite_epdc_fill(struct pl_epdc *p, const struct pl_area *area,
                         uint8_t g)
{
    struct it8951 *it8951 = p->data;
    return it8951_fill(it8951, area, g);
}

static int ite_load_image(struct pl_epdc *p, const char *path,
                          struct pl_area *area, int left, int top)
{
    struct it8951 *it8951 = p->data;
    return it8951_load_image(it8951, path, 0, 8, area, left, top);
}

int ite_epdc_init(struct pl_epdc *epdc, const struct pl_dispinfo *dispinfo,
                  struct it8951 *it8951, struct vcom_cal *vcom_cal)
{

    int stat;
    stat = 0;

    assert(epdc != NULL);
    assert(dispinfo != NULL);
    assert(it8951 != NULL);
    assert(it8951->data != NULL);

    if (it8951->data->hrdy != PL_GPIO_NONE)
        LOG("Using HRDY GPIO");

    it8951->flags.needs_update = 0;

    epdc->clear_init = ite_epdc_clear_init;
    epdc->update = ite_epdc_update;
    epdc->wait_update_end = ite_epdc_wait_update_end;
    epdc->set_power = ite_epdc_set_power;
    epdc->set_epd_power = ite_epdc_set_epd_power;
    epdc->data = it8951;
    epdc->dispinfo = dispinfo;
    epdc->update_temp = ite_epdc_update_temp;
    epdc->fill = ite_epdc_fill;
    epdc->load_image = ite_load_image;

    if (stat)
        return -1;

    it8951_load_init_code(it8951);

    it8951_update_Temp(it8951, epdc->temp_mode, 23);

    epdc->xres = it8951->xres;
    epdc->yres = it8951->yres;

    const struct pl_dispinfo_info *info = &(epdc->dispinfo->info);

    it8951_setVcom(it8951, vcom_calculate(vcom_cal, info->vcom));

    //Turn off pmic manual, cause set VCom turns HVs on
    it8951_set_epd_power(it8951, 0);

    LOG("Ready %dx%d", epdc->xres, epdc->yres);

    return 0;
}

