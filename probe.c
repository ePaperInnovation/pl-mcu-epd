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
#include <pl/platform.h>
#include <pl/hwinfo.h>
#include <string.h>
#include <stdio.h>
#include "probe.h"
#include "assert.h"
#include "config.h"
#include "i2c-eeprom.h"
#include "pnm-utils.h"
#include "plwf.h"
#include "vcom.h"

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
static const struct i2c_eeprom g_disp_eeprom = {
	&plat->i2c_host, I2C_PLWF_EEPROM_ADDR, EEPROM_24AA256
};
#endif

#define PLATFORM_PATH "0:/"
#ifdef CONFIG_DISPLAY_TYPE
#define DISPLAY_PATH PLATFORM_PATH""CONFIG_DISPLAY_TYPE
static const char g_display_path[] = DISPLAY_PATH;
#endif

#if !CONFIG_DISP_DATA_SD_ONLY
static int load_wf_eeprom(struct platform *plat, struct plwf *plwf);
#endif
#if !CONFIG_DISP_DATA_EEPROM_ONLY
static int load_wf_sdcard(struct platform *plat, struct plwf *plwf);
#endif

int probe(struct platform *plat, const struct pl_hw_info *pl_hw_info,
	  struct s1d135xx *s1d135xx)
{
#if !CONFIG_DISP_DATA_SD_ONLY
	const struct i2c_eeprom *pe = &g_disp_eeprom;
#else
	const struct i2c_eeprom *pe = NULL;
#endif
	char full_path[10];
	struct pl_epdc *epdc = &plat->epdc;

	/* ToDo: This should be either in platform or main */
	struct plwf plwf;
	struct vcom_cal vcomcal;
	static struct tps65185_info *pmic_info;

	int stat;

	/* -- Configure the I2C bus again if we use an external bridge -- */

	switch (pl_hw_info->board.i2c_mode) {
	case I2C_MODE_HOST: /* MSP430, I2C already initialised */
		stat = 0;
		break;
	case I2C_MODE_DISP: /* This must be the Epson S1D13541 */
	case I2C_MODE_S1D13524:
#if 1
		LOG("Epson I2C master not tested yet.");
		stat = -1;
#else
		stat = epson_i2c_init(epson, &i2c);
#endif
		break;
	case I2C_MODE_SC18IS6XX:
	case I2C_MODE_NONE:
		LOG("I2C_MODE not supported");
		stat = -1;
		break;
	default:
		abort_msg("Invalid I2C mode");
	}

	if (stat)
		return -1;

	/* -- Load the display data -- */

#if CONFIG_DISP_DATA_EEPROM_ONLY
	stat = load_wf_eeprom(plat, &plwf);
#elif CONFIG_DISP_DATA_SD_ONLY
	stat = load_wf_sdcard(plat, &plwf);
#elif CONFIG_DISP_DATA_EEPROM_SD
	stat = load_wf_eeprom(plat, &plwf) || load_wf_sdcard(plat, &plwf);
#elif CONFIG_DISP_DATA_SD_EEPROM
	stat = load_wf_sdcard(plat, &plwf) || load_wf_eeprom(plat, &plwf);
#endif
	if (stat) {
		LOG("Failed to load display data");
		return -1;
	}

	plwf_log(&plwf.data);

	/* -- Initialise the VCOM and HV-PMIC -- */

	vcom_init(&vcomcal, &pl_hw_info->vcom);
	tps65185_init(&plat->host_i2c, I2C_PMIC_ADDR, &pmic_info);
	tps65185_configure(pmic_info, &vcomcal);
	tps65185_set_vcom_voltage(pmic_info, plwf.data.info.vcom);

#if S1D135XX_INTERIM
	s1d135xx->epson = &g_epson;
	epsonif_hack(&plat->gpio, s1d135xx->data);
#endif

	/* -- Initialise the EPDC controller -- */

	switch (pl_hw_info->board.epdc_ref) {
	case EPDC_S1D13524:
		stat = epson_epdc_init(epdc, &plwf.data,
				       EPSON_EPDC_S1D13524, s1d135xx);
		break;
	case EPDC_S1D13541:
		stat = epson_epdc_init(epdc, &plwf.data,
				       EPSON_EPDC_S1D13541, s1d135xx);
		break;
	case EPDC_NONE:
#if PL_EPDC_STUB
		stat = pl_epdc_stub_init(epdc);
		break;
#endif
	default:
		abort_msg("Invalid EPDC identifier");
	}

	if (stat) {
		LOG("Failed to initialise EPDC");
		return -1;
	}

#if 0 /* enable during development of new EPDC implementations */
	assert(epdc->init != NULL);
	assert(epdc->load_wf_lib != NULL);
	assert(epdc->update != NULL);
	assert(epdc->wait_update_end != NULL);
	assert(epdc->set_power != NULL);
	assert(epdc->set_temp_mode != NULL);
	assert(epdc->update_temp != NULL);
	assert(epdc->fill != NULL);
#endif

	return 0;
}

#if !CONFIG_DISP_DATA_SD_ONLY
static int load_wf_eeprom(struct platform *plat, struct plwf *plwf)
{
	LOG("Loading display data from EEPROM");

	LOG("Not tested that yet.");

	return -1;
}
#endif

#if !CONFIG_DISP_DATA_EEPROM_ONLY
static int load_wf_sdcard(struct platform *plat, struct plwf *plwf)
{
	FIL vcom_file;
	int stat;

	LOG("Loading display data from SD card");

	if (f_chdir(g_display_path) != FR_OK)
		return -1;

	plwf->mode = PLWF_MODE_SD_CARD;
	plwf->eeprom = NULL;
	plwf->path = g_display_path;

	plwf->data.vermagic.magic = PL_DATA_MAGIC;
	plwf->data.vermagic.version = PL_DATA_VERSION;
	plwf->data.info.panel_id[0] = '\0';
	strncpy(plwf->data.info.panel_type, CONFIG_DISPLAY_TYPE,
		sizeof plwf->data.info.panel_type);

	if (f_open(&vcom_file, "display/vcom", FA_READ) != FR_OK) {
		LOG("VCOM file not found");
		return -1;
	}

	stat = pnm_read_int32(&vcom_file, &plwf->data.info.vcom);
	f_close(&vcom_file);

	if (stat) {
		LOG("Failed to read VCOM");
		return -1;
	}

	LOG("vcom: %ld", plwf->data.info.vcom);

	memset(plwf->data.info.waveform_md5, 0xFF,
	       sizeof plwf->data.info.waveform_md5);

	plwf->data.info.waveform_full_length = 0;
	plwf->data.info.waveform_lzss_length = 0;
	plwf->data.info.waveform_id[0] = '\0';
	plwf->data.info.waveform_target[0] = '\0';
	plwf->data.info_crc = 0xFFFF;

	return 0;
}
#endif
