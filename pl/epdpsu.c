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
 * epdpsu.c -- EPD PSU generic implementation
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/epdpsu.h>
#include <pl/gpio.h>
#include <pl/epdc.h>
#include "assert.h"

#define LOG_TAG "epdpsu"
#include "utils.h"

/* Set to 1 to enable verbose logging */
#define LOG_VERBOSE 0

/* --- GPIO --- */

static int pl_epdpsu_gpio_on(struct pl_epdpsu *psu)
{
	struct pl_epdpsu_gpio *p = psu->data;
	unsigned timeout;

	if (psu->state)
		return 0;

#if LOG_VERBOSE
	LOG("on");
#endif

	pl_gpio_set(p->gpio, p->hv_en, 1);

	for (timeout = p->timeout_ms; timeout; timeout--) {
		if (pl_gpio_get(p->gpio, p->pok))
			break;
		mdelay(1);
	}

	if (!timeout) {
		LOG("POK timeout");
		pl_gpio_set(p->gpio, p->hv_en, 0);
		return -1;
	}

	pl_gpio_set(p->gpio, p->com_close, 1);
	msleep(p->on_delay_ms);
	psu->state = 1;

	return 0;
}

static int pl_epdpsu_gpio_off(struct pl_epdpsu *psu)
{
	struct pl_epdpsu_gpio *p = psu->data;

#if LOG_VERBOSE
	LOG("off");
#endif

	pl_gpio_set(p->gpio, p->com_close, 0);
	pl_gpio_set(p->gpio, p->hv_en, 0);
	msleep(p->off_delay_ms);
	psu->state = 0;

	return 0;
}

int pl_epdpsu_gpio_init(struct pl_epdpsu *psu, struct pl_epdpsu_gpio *p)
{
	assert(psu != NULL);
	assert(p != NULL);
	assert(p->gpio != NULL);
	assert(p->timeout_ms != 0);
	assert(p->hv_en != PL_GPIO_NONE);
	assert(p->com_close != PL_GPIO_NONE);
	assert(p->pok != PL_GPIO_NONE);
	assert(p->flt != PL_GPIO_NONE);

	psu->on = pl_epdpsu_gpio_on;
	psu->off = pl_epdpsu_gpio_off;
	psu->state = 0;
	psu->data = p;

	return 0;
}

/* --- EPDC --- */

static int pl_epdpsu_epdc_on(struct pl_epdpsu *psu)
{
	struct pl_epdc *epdc = psu->data;

	if (!psu->state) {
		if (epdc->set_epd_power(epdc, 1))
			return -1;

		psu->state = 1;
	}

	return 0;
}

static int pl_epdpsu_epdc_off(struct pl_epdpsu *psu)
{
	struct pl_epdc *epdc = psu->data;

	if (psu->state) {
		if (epdc->set_epd_power(epdc, 0))
			return -1;

		psu->state = 0;
	}

	return 0;
}

int pl_epdpsu_epdc_init(struct pl_epdpsu *psu, struct pl_epdc *epdc)
{
	psu->on = pl_epdpsu_epdc_on;
	psu->off = pl_epdpsu_epdc_off;
	psu->state = 0;
	psu->data = epdc;

	return 0;
}

#if PL_EPDPSU_STUB

/* --- Stub --- */

static int pl_epdpsu_stub_on(struct pl_epdpsu *psu)
{
#if LOG_VERBOSE
	LOG("stub on");
#endif

	psu->state = 1;

	return 0;
}

static int pl_epdpsu_stub_off(struct pl_epdpsu *psu)
{
#if LOG_VERBOSE
	LOG("stub off");
#endif

	psu->state = 0;

	return 0;
}

int pl_epdpsu_stub_init(struct pl_epdpsu *psu)
{
	assert(psu != NULL);

	psu->on = pl_epdpsu_stub_on;
	psu->off = pl_epdpsu_stub_off;
	psu->state = 0;
	psu->data = NULL;

	return 0;
}

#endif /* PL_EPDPSU_STUB */
