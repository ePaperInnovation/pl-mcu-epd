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
 * S1D13524.c -- Epson S1D13524 specific controller functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#if 0
#include <stdlib.h>
#include "types.h"
#include "assert.h"
#include "S1D13524.h"
#include "epson-cmd.h"
#include "epson-utils.h"

#define EPSON_CODE_PATH	"bin/Ecode.bin"
#define WAVEFORM_PATH	"display/waveform.bin"

void dump_reg(short reg)
{
	uint16_t data;

	epson_reg_read(reg, &data);
	printk("524: Reg:0x%04x => 0x%04x\n", reg, data);
}

/* Initialise the S1D13524 controller
 *
 */
int s1d13524_init(screen_t screen, struct s1d135xx **controller)
{
	uint16_t PLLCFG0, PLLCFG1,PLLCFG2, PLLCFG3;
	uint16_t WFadd1,WFadd0;
	uint16_t rev, conf;
	uint16_t readval;
	screen_t previous;
	struct s1d135xx *epson;

	*controller = epson = (struct s1d135xx *)malloc(sizeof(struct s1d135xx));
	if (NULL == epson)
		return -ENOMEM;

	epson_set_idle_mask(0x0020, 0x0000);

	epson->screen = screen;
	epson->temp_mode = TEMP_MODE_UNDEFINED;
	epson->power_mode = PWR_STATE_UNDEFINED;

	if (epsonif_claim(0, screen, &previous) < 0)
		return -EIO;

	epson_softReset();	// reset the controller

	epson_wait_for_idle();

	// read the controller id and configuration information
	epson_reg_read(PROD_CODE_REG, &epson->id);
	check(epson->id == 0x004F);
	epson_reg_read(REV_CODE_REG, &rev);
	epson_reg_read(CONF_PIN_RD_VAL_REG, &conf);
	printk("524: id:0x%04x, rev:0x%04x, conf: 0x%04x\n", epson->id, rev, conf);

	// configure the clocks
	PLLCFG0 = 0x340F; 	/* 	PLL charge pump current select = 8
					    	PLL output frequency select = 400Mhz
					    	PLL frequency multiplier select = 16 */
	PLLCFG1 = 0x0300; 	/*  M Divider bit = 2 */
	PLLCFG2 = 0x1680;	/*  default value for DLL TX */
	PLLCFG3 = 0x1880;	/*  default value for DLL RX */
	epson_cmd_p4(INIT_PLL, PLLCFG0, PLLCFG1, PLLCFG2, PLLCFG3);
	epson_wait_for_idle();

	// load the Epson controller code
	if(epson_loadEpsonCode(EPSON_CODE_PATH) < 0)
		return (-ENOENT);

	epson_cmd_p1(INIT_SYS_RUN, 0x0000);
	epson->power_mode = PWR_STATE_RUN;
	epson_wait_for_idle();
	// A side effect of the INIT_SYS_RUN is that the power up
	// sequence runs. For now run power down sequence here.
	epson_power_down();

	// Check that the controller code loaded correctly
	epson_reg_read(CMD_SEQ_AUTOBOOT_CMD_REG, &readval);
	if ((readval & (uint16_t)(1 << 15)) == 0) {
		printk(KERN_ERR "524: Init code checksum error!\n");
		return -EIO;
	}

	// read the address that waveform data should be loaded to
	epson_reg_read(WAVE_ADD_REG1, &WFadd1);
	epson_reg_read(WAVE_ADD_REG0, &WFadd0);

	// waveform will be loaded into SDRAM
	epson_reg_write(WAVEFORM_RD_CONF_REG,0x8001);

	/* send the Waveform file */
	if (epson_loadEpsonWaveform(WAVEFORM_PATH, (u32)(((u32)WFadd1 << 16L) | WFadd0)) < 0)
		return -ENOENT;

	// Tell the controller to process the waveform data
	epson_cmd_p2(RD_WFM_INFO, WFadd0, WFadd1);
	epson_wait_for_idle();

	/* Need to check checksum and status bits after waveform load */
	epson_reg_read(DISPLAY_BUSY_REG, &readval);
	if (readval & 0x1F00) {
		printk(KERN_ERR "524: Waveform checksum/format error!\n");
		return -EIO;
	}

	// disable auto temperature sensing
	epson_reg_write(TEMP_AUTO_RETRIEVE_REG, 0x0001);

	epson_cmd_p0(UPD_GDRV_CLR);
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();

	// setup rotation (none)
	epson_cmd_p1(INIT_HOST_ROT_MODE, 0x0000);
	epson_wait_for_idle();

	// manual operation, greyscale
	epson_cmd_p2(INIT_CTLR_MODE, 0x0200, 0x4002);
	epson_wait_for_idle();

	epson_reg_read(DISPLAY_FRAME_DATA_LG_REG, &epson->xres);
	epson_reg_read(DISPLAY_PNL_BP_DATA_LG_REG, &epson->yres);

	// fill the image buffer with white data
	epson_fill_buffer(0x0000, true, epson->yres, epson->xres, 0xff);
	epson_cmd_p1(UPD_INIT, 0x0000);
	epson_wait_for_idle();

	// UPD_INIT turns on automatic updates as a side effect
	// restore manual operation, greyscale
	epson_cmd_p2(INIT_CTLR_MODE, 0x0200, 0x4002);
	epson_wait_for_idle();

	epsonif_release(0, previous);

	return 0;
}


int s1d13524_update_display(struct s1d135xx *epson, int waveform)
{
	uint16_t reg0;
	uint16_t reg1;

	assert(epson);

	epson_cmd_p1(UPD_FULL, (waveform << 8));
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_FREND);
	epson_wait_for_idle();

	// there may be a bit set here indicating out of range temperature
	epson_reg_read(INT_RAW_STATUS_REG, &reg0);
	epson_reg_read(DISPLAY_INT_RAW_STAT_REG, &reg1);
	printk("524: RAW_INT: 0x%04x, DISPLAY_INT_RAW: 0x%04x\n", reg0, reg1);
	epson_reg_write(INT_RAW_STATUS_REG, 0xffff);
	epson_reg_write(DISPLAY_INT_RAW_STAT_REG, 0xffff);

	return 0;
}

/* Configure controller for specified temperature mode */
int s1d13524_set_temperature_mode(struct s1d135xx *epson,
				  enum s1d135xx_temp_mode temp_mode)
{
	assert(epson);

	switch(temp_mode)
	{
	case TEMP_MODE_MANUAL:
		// disable auto temperature sensing
		epson_reg_write(TEMP_AUTO_RETRIEVE_REG, 0x0001);
		break;
	case TEMP_MODE_EXTERNAL:
		// enable auto temperature sensing
		epson_reg_write(TEMP_AUTO_RETRIEVE_REG, 0x0000);
		break;
		// no internal sensor on 524
	case TEMP_MODE_INTERNAL:
	case TEMP_MODE_UNDEFINED:
	default:
		return -EPARAM;
	}

	epson->temp_mode = temp_mode;

	return 0;
}

int s1d13524_set_temperature(struct s1d135xx *epson, s8 temp)
{
	assert(epson);

	epson->temp_set = temp;
	return 0;
}

int s1d13524_get_temperature(struct s1d135xx *epson, s8 *temp)
{
	assert(epson);

	*temp = epson->temp_measured;

	return 0;
}

int s1d13524_measure_temperature(struct s1d135xx *epson)
{
	int8_t temp_measured;
	uint16_t reg;

	assert(epson);

	switch(epson->temp_mode)
	{
	case TEMP_MODE_MANUAL:
		// apply manually specified temperature
		epson_reg_write(TEMP_VALUE_REG, (epson->temp_set & 0x00ff));
		temp_measured = epson->temp_set;
		break;
	case TEMP_MODE_EXTERNAL:
		epson_reg_read(0x0216, &reg);
		temp_measured = (reg & 0x00ff);
		break;
	case TEMP_MODE_INTERNAL:
	case TEMP_MODE_UNDEFINED:
	default:
		return -1;
	}

	epson->temp_measured = temp_measured;

	printk("524: Temperature: %d\n", temp_measured);

	return 0;
}

#if 0
void setup_auto_temp(u8 i2c_addr)
{
	short status;
	short temp;

	// setup i2c address and register to read
	epson_reg_write(0x0210, (i2c_addr << 8) | 0x00);

	// disable temperature sensing in PMIC reg 1, bit 0=1
	epson_reg_write(0x0214, (0x0100 | (1 << 2)) | 0x0003);
	do {
		epson_reg_read(0x0212, &status);
	} while (status & 0x0001);
#if 0
	// enable temperature sensing in PMIC reg 1, bit 0=0
	epson_reg_write(0x0214, (0x0100 | (0 << 2)) | 0x0003);
	do {
		epson_reg_read(0x0212, &status);
	} while (status & 0x0001);
#endif
	// trigger read
	epson_reg_write(0x0214, 0x0001);
	// look at status until not busy
	do {
		epson_reg_read(0x0212, &status);
	} while (status & 0x0001);
	// read temp value accquired from sensor
	epson_reg_read(0x0216, &temp);

	// enable auto temperature sensing
	epson_reg_write(0x0320, 0x0000);

}
#endif
#endif /* 0 */
