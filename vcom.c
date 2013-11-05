/*
 * Copyright (C) 2013 Plastic Logic Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * vcom.c -- VCOM Calculation support
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "types.h"
#include "assert.h"
#include "vcom.h"

struct vcom_cal {
	struct vcom_info vcom;	/* Variable Power board calibration data */
	s32 swing;
	s32 swing_ideal;
 	s32 dac_offset;
 	s32 dac_dx;
 	s32 dac_dy;
	s32 dac_step_mv;
};

static struct vcom_cal vcom;

int vcom_init(struct vcom_info *c, s32 vgswing_ideal, struct vcom_cal **p)
{
	struct vcom_cal *v = &vcom;

	assert(c);
	assert(p);

	memcpy(&v->vcom, c, sizeof(struct vcom_info));
	v->dac_dx = c->dac_x2 - c->dac_x1;
	v->dac_dy = c->dac_y2 - c->dac_y1;
	v->dac_offset = c->dac_y1 - DIV_ROUND_CLOSEST((c->dac_x1 * v->dac_dy),  v->dac_dx);
	v->swing = c->vgpos_mv - c->vgneg_mv;
	v->swing_ideal = vgswing_ideal;
	v->dac_step_mv = DIV_ROUND_CLOSEST(v->dac_dy, v->dac_dx);

	*p = v;

	return 0;
}


int vcom_calculate(struct vcom_cal *p, int mv)
{
	s32 mv_scaled;
	int dac_value;

	assert(p);

	mv_scaled = DIV_ROUND_CLOSEST(mv * p->swing, p->swing_ideal);
	dac_value = DIV_ROUND_CLOSEST((mv_scaled - p->dac_offset) * p->dac_dx, p->dac_dy);

	printk("vcom: mv(input):%d, mv(target):%ld, Reg:(%d)\n", mv, mv_scaled, dac_value);

	return dac_value;
}
