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
 * probe.c -- Probing the hardware
 *
 * Authors:
 *    Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <epson/epson-epdc.h>
#include <epson/epson-i2c.h>
#include <pl/platform.h>
#include <pl/hwinfo.h>
#include <pl/dispinfo.h>
#include <pl/wflib.h>
#include <string.h>
#include <stdio.h>
#include "probe.h"
#include "assert.h"
#include "config.h"
#include "i2c-eeprom.h"
#include "pnm-utils.h"
#include "vcom.h"
#include "pmic-tps65185.h"

#define LOG_TAG "probe"
#include "utils.h"

#define I2C_PLWF_EEPROM_ADDR 0x54
#define I2C_PMIC_ADDR        0x68

#include "epson/epson-s1d135xx.h"

#if S1D135XX_INTERIM
#include "epson/S1D135xx.h"
struct _s1d135xx g_epson;
#endif

#if !CONFIG_DISP_DATA_SD_ONLY
static struct i2c_eeprom g_disp_eeprom = {
	NULL, I2C_PLWF_EEPROM_ADDR, EEPROM_24AA256
};
#endif

/* ToDo: move to platform or main */
static struct pl_dispinfo g_dispinfo;

#if !CONFIG_DISP_DATA_SD_ONLY
static struct pl_wflib_eeprom g_wflib_ctx;
#endif

int probe(struct platform *plat, const struct pl_hw_info *pl_hw_info,
	  struct s1d135xx *s1d135xx)
{
	struct pl_epdc *epdc = &plat->epdc;

	/* ToDo: This should be either in platform or main */
	struct vcom_cal vcomcal;
	static struct tps65185_info *pmic_info;

	int stat;

	/* -- Configure the I2C bus again if we use an external bridge -- */

	switch (pl_hw_info->board.i2c_mode) {
	case I2C_MODE_HOST: /* MSP430, I2C already initialised */
		LOG("I2C: Host");
		stat = 0;
		plat->i2c = &plat->host_i2c;
		break;
	case I2C_MODE_DISP: /* This must be the Epson S1D13541... */
		LOG("I2C: S1D13541");
		stat = epson_i2c_init(s1d135xx, &plat->disp_i2c,
				      EPSON_EPDC_S1D13541);
		plat->i2c = &plat->disp_i2c;
		break;
	case I2C_MODE_S1D13524:
		LOG("I2C: S1D13524");
		stat = epson_i2c_init(s1d135xx, &plat->disp_i2c,
				      EPSON_EPDC_S1D13524);
		plat->i2c = &plat->disp_i2c;
		break;
	case I2C_MODE_SC18IS6XX:
		LOG("I2C: not supported");
		stat = -1;
		break;
	case I2C_MODE_NONE:
	default:
		abort_msg("Invalid I2C mode");
	}

	if (stat)
		return -1;

	/* -- Load the display data -- */

#if !CONFIG_DISP_DATA_SD_ONLY
	g_disp_eeprom.i2c = plat->i2c;
#endif

#if CONFIG_DISP_DATA_EEPROM_ONLY
	stat = pl_dispinfo_init_eeprom(&g_dispinfo, &g_disp_eeprom);
#elif CONFIG_DISP_DATA_SD_ONLY
	stat = pl_dispinfo_init_fatfs(&g_dispinfo);
#elif CONFIG_DISP_DATA_EEPROM_SD
	stat = (pl_dispinfo_init_eeprom(&g_dispinfo, &g_disp_eeprom) ||
		pl_dispinfo_init_fatfs(&g_dispinfo));
#elif CONFIG_DISP_DATA_SD_EEPROM
	stat = (pl_dispinfo_init_fatfs(&g_dispinfo) ||
		pl_dispinfo_init_eeprom(&g_dispinfo, &g_disp_eeprom));
#endif
	if (stat) {
		LOG("Failed to load display data");
		return -1;
	}

	pl_dispinfo_log(&g_dispinfo);

#if !CONFIG_DISP_DATA_SD_ONLY
	if (pl_wflib_init_eeprom(plat->epdc.wflib, &g_wflib_ctx,
				 &g_disp_eeprom, &g_dispinfo))
		return -1;
#endif

	/* -- Initialise the VCOM and HV-PMIC -- */

	vcom_init(&vcomcal, &pl_hw_info->vcom);

	switch (pl_hw_info->board.hv_pmic) {
	case HV_PMIC_NONE:
		LOG("HV-PMIC: None");
		stat = 0;
		break;
	case HV_PMIC_MAX17135:
		LOG("HV-PMIC: MAX17135");
		abort_msg("Not verified yet");
		stat = -1;
		break;
	case HV_PMIC_TPS65185:
		LOG("HV-PMIC: TPS65185");
		stat = tps65185_init(plat->i2c, I2C_PMIC_ADDR, &pmic_info,
				     &vcomcal);
		if (!stat) /* ToDo: generalise set_vcom with HV-PMIC API */
			tps65185_set_vcom_voltage(pmic_info,
						  g_dispinfo.info.vcom);
		break;
	default:
		abort_msg("Invalid HV-PMIC id");
	}

	if (stat) {
		LOG("Failed to initialise HV-PMIC");
		return -1;
	}

#if S1D135XX_INTERIM
	s1d135xx->epson = &g_epson;
	epsonif_hack(&plat->gpio, s1d135xx->data);
#endif

	/* -- Initialise the EPDC controller -- */

	switch (pl_hw_info->board.epdc_ref) {
	case EPDC_S1D13524:
		LOG("EPDC: S1D13524");
		stat = epson_epdc_init(epdc, &g_dispinfo,
				       EPSON_EPDC_S1D13524, s1d135xx);
		break;
	case EPDC_S1D13541:
		LOG("EPDC: S1D13541");
		stat = epson_epdc_init(epdc, &g_dispinfo,
				       EPSON_EPDC_S1D13541, s1d135xx);
		break;
	case EPDC_NONE:
#if PL_EPDC_STUB
		LOG("EPDC: Stub");
		stat = pl_epdc_stub_init(epdc);
		break;
#endif /* fall through otherwise */
	default:
		abort_msg("Invalid EPDC identifier");
	}

	if (stat) {
		LOG("Failed to initialise EPDC");
		return -1;
	}

#if 0 /* enable during development of new EPDC implementations */
	assert(epdc->init != NULL);
	assert(epdc->load_wflib != NULL);
	assert(epdc->update != NULL);
	assert(epdc->wait_update_end != NULL);
	assert(epdc->set_power != NULL);
	assert(epdc->set_temp_mode != NULL);
	assert(epdc->update_temp != NULL);
	assert(epdc->fill != NULL);
	assert(epdc->load_image != NULL);
#endif

	return 0;
}
