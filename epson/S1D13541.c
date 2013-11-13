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
 * S1D13541.c -- Epson S1D13541 controller specific functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <stddef.h>
#include <stdlib.h>
#include "types.h"
#include "assert.h"
#include "S1D13541.h"
#include "epson-cmd.h"
#include "epson-utils.h"
#include "i2c-eeprom.h"
#include "plwf.h"

#define LOG(msg, ...) printf("S1D13541: "msg"\n", ##__VA_ARGS__)

#define EPSON_CODE_PATH	"bin/Ecode.bin"
#define WAVEFORM_PATH	"display/waveform.bin"

#define KEY 0x12345678
#define KEY_1 0x1234
#define	KEY_2 0x5678

#define GATE_POWER_BEFORE_INIT 1

/* These need changing to use our definitions */
#define TIMEOUT_MS 5000

#define REG_CLOCK_CONFIGURATION    ((uint16_t)0x0010)

#define BF_INIT_CODE_CHECKSUM	   ((uint16_t)(1 << 15))
#define INT_WF_CHECKSUM_ERROR	   ((uint16_t)(1 << 12))
#define INT_WF_INVALID_FORMAT	   ((uint16_t)(1 << 11))
#define INT_WF_CHECKSUM_ERROR      ((uint16_t)(1 << 12))
#define INT_WF_OVERFLOW            ((uint16_t)(1 << 13))

/* 541 register bit field value list */
#define INTERNAL_CLOCK_ENABLE      ((uint16_t)(1 << 7))
#define INIT_CODE_CHECKSUM_ERROR   ((uint16_t)(0 << 15))

/* 541 constant value list */
#define PRODUCT_CODE 0x0053

static int wait_for_HRDY_ready(int timeout)
{
	epson_wait_for_idle();
	return 0;
}

static int delay_for_HRDY_ready(int timeout)
{
	epson_wait_for_idle();
	return 0;
}

static int send_init_code()
{
	uint16_t readval;

	if (epson_loadEpsonCode(EPSON_CODE_PATH) < 0)
		return -EIO;

	epson_reg_read(CMD_SEQ_AUTOBOOT_CMD_REG, &readval);
	if ((readval & BF_INIT_CODE_CHECKSUM) == INIT_CODE_CHECKSUM_ERROR) {
		LOG("init code checksum error");
		return -EIO;
	}

	return 0;
}

static int send_waveform(void)
{
	uint16_t readval;
	int stat;

	if (epson_loadEpsonWaveform(WAVEFORM_PATH, S1D13541_WF_ADDR) < 0)
		return -EIO;

	stat = delay_for_HRDY_ready(TIMEOUT_MS);

	if (stat) {
		LOG("failed to send waveform");
		return stat;
	}

	epson_reg_read(DISPLAY_INT_RAW_STAT_REG, &readval);

	if (readval & INT_WF_INVALID_FORMAT) {
		LOG("invalid waveform format");
		return -EIO;
	}

	if (readval & INT_WF_CHECKSUM_ERROR) {
		LOG("waveform checksum error");
		return -EIO;
	}

	if (readval & INT_WF_OVERFLOW) {
		LOG("waveform overflow");
		return -EIO;
	}

	return 0;
}

int s1d13541_init_display(struct s1d135xx *epson)
{
	assert(epson);

	epson_cmd_p0(UPD_INIT);
	epson_wait_for_idle();
	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();

	return 0;
}

int s1d13541_update_display(struct s1d135xx *epson, int waveform)
{
	assert(epson);

	epson_cmd_p1(UPD_FULL, WAVEFORM_MODE(waveform) | UPDATE_LUT(0));
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();

	return 0;
}

int s1d13541_update_display_area(struct s1d135xx *epson, int waveform,
				 const struct area *area)
{
	assert(epson != NULL);

	epson_cmd_p5(UPD_FULL_AREA, WAVEFORM_MODE(waveform),
		     (area->left & S1D135XX_XMASK),
		     (area->top & S1D135XX_YMASK),
		     (area->width & S1D135XX_XMASK),
		     (area->height & S1D135XX_YMASK));
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();

	return 0;
}

int s1d13541_wait_update_end(struct s1d135xx *epson)
{
	epson_cmd_p0(WAIT_DSPE_FREND);
	epson_wait_for_idle();

	return 0;
}

int s1d13541_init_start(screen_t screen, screen_t *previous,
			struct s1d135xx **controller)
{
	struct s1d135xx *epson;

	LOG("sizeof(temp_mode): %lu", sizeof(enum s1d135xx_pwr_state));
	*controller = epson = malloc(sizeof(struct s1d135xx));
	if (NULL == epson)
		return -ENOMEM;

	epson_wait_for_idle_mask(0x2000, 0x2000);

	epson->screen = screen;

	epson->keycode1 = KEY_2;
	epson->keycode2 = KEY_1;
	epson->temp_mode = TEMP_MODE_UNDEFINED;
	epson->power_mode = PWR_STATE_UNDEFINED;

	if (epsonif_claim(0, screen, previous) < 0)
		return -EIO;

	return 0;
}

int s1d13541_init_prodcode(struct s1d135xx *epson)
{
	int retval = 0;
	uint16_t product;

	assert(epson);

	epson_softReset();

	retval = wait_for_HRDY_ready(TIMEOUT_MS);

	epson_reg_read(PROD_CODE_REG, &product);

	printk(KERN_INFO "541: product code = 0x%04x\n", product);
	if (product != PRODUCT_CODE) {
		printk(KERN_ERR "541: invalid product code\n");
		retval = -EIO;
	}

	return retval;
}

int s1d13541_init_clock(struct s1d135xx *epson)
{
	int retval = 0;

	assert(epson);

	epson_reg_write(REG_CLOCK_CONFIGURATION, INTERNAL_CLOCK_ENABLE);
	msleep(10);
	retval = wait_for_HRDY_ready(TIMEOUT_MS);
	if (retval != 0) {
		printk(KERN_ERR "541: clock enable failed\n");
	}

	return retval;
}

int s1d13541_init_initcode(struct s1d135xx *epson)
{
	int retval = 0;

	assert(epson);

	retval = send_init_code();
	if (retval != 0) {
		printk(KERN_ERR "541: send_init_code failed\n");
	}

	return retval;
}

int s1d13541_init_pwrstate(struct s1d135xx *epson)
{
	int retval = 0;

	assert(epson);

	epson_cmd_p0(INIT_SYS_STBY);
	epson->power_mode = PWR_STATE_STANDBY;
	msleep(100);

	retval = wait_for_HRDY_ready(TIMEOUT_MS);
	if (retval != 0) {
		printk(KERN_ERR "541: init and standby failed\n");
	}

	return retval;
}

int s1d13541_init_keycode(struct s1d135xx *epson)
{
	int retval = 0;

	assert(epson);

	epson_reg_write(REG_PROTECTION_KEY_1, epson->keycode1);
	epson_reg_write(REG_PROTECTION_KEY_2, epson->keycode2);
	retval = wait_for_HRDY_ready(TIMEOUT_MS);
	if (retval != 0) {
		printk(KERN_ERR "541: write keycode failed\n");
	}

	return retval;
}

int s1d13541_init_waveform_sd(struct s1d135xx *epson)
{
	int retval = 0;

	assert(epson);

	retval = send_waveform();
	if (retval != 0)
		printk(KERN_ERR "541: Waveform load from filesystem failed\n");

	return retval;
}

int s1d13541_init_gateclr(struct s1d135xx *epson)
{
	int retval = 0;

	assert(epson);

#if GATE_POWER_BEFORE_INIT
	epson_mode_run(epson);
	epson_cmd_p0(UPD_GDRV_CLR);
	epson_wait_for_idle();
#else
	epson_cmd_p0(UPD_GDRV_CLR);
	epson_wait_for_idle();
	epson_mode_run(epson);
#endif
	epson_cmd_p0(WAIT_DSPE_TRG);
	retval = wait_for_HRDY_ready(TIMEOUT_MS);
	if (retval != 0) {
		printk(KERN_ERR "541: clear gate driver failed\n");
	}
	// init_rot_mode
	// not required in this case

#if 0
	// fill buffer with blank image using H/W fill support
	epson_reg_read(HOST_MEM_CONF_REG, &readval);
	epson_reg_write(HOST_MEM_CONF_REG, (readval &= ~1));
	epson_reg_write(DISPLAY_UPD_BUFF_PXL_VAL_REG, 0x00f0);
	epson_reg_write(DISPLAY_CTRL_TRIG_REG, 3); // trigger buffer init
	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle(); 		// wait for it to complete

	// update the current pixel data - no display update occurs
	epson_cmd_p0(UPD_INIT);
	epson_wait_for_idle();
	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();
#endif

	return retval;
}

int s1d13541_init_end(struct s1d135xx *epson, screen_t previous)
{
	assert(epson);

	// get x and y definitions.
	epson_reg_read(REG_LINE_DATA_LENGTH, &epson->xres);
	epson_reg_read(REG_FRAME_DATA_LENGTH, &epson->yres);

	epsonif_release(0, previous);
	return 0;
}

int s1d13541_send_waveform(void)
{
	return send_waveform();
}

int s1d13541_set_temperature_mode(struct s1d135xx *epson,
				  enum s1d135xx_temp_mode temp_mode)
{
	uint16_t reg;

	assert(epson);

	epson_reg_read(PERIPHERAL_CONFIG_REG, &reg);

	reg &= ~TEMP_SENSOR_CONTROL;

	switch(temp_mode)
	{
	case TEMP_MODE_MANUAL:
		break;
	case TEMP_MODE_INTERNAL:
		reg &= ~TEMP_SENSOR_EXTERNAL;
		break;
	case TEMP_MODE_EXTERNAL:
		reg |= TEMP_SENSOR_EXTERNAL;
		break;
	case TEMP_MODE_UNDEFINED:
	default:
		return -EPARAM;
	}

	epson_reg_write(PERIPHERAL_CONFIG_REG, reg );

	epson->temp_mode = temp_mode;

	/* Configure the controller to check for waveform update after temperature sense
	 */
	epson_reg_read(REG_WAVEFORM_DECODER_BYPASS, &reg);
	epson_reg_write(REG_WAVEFORM_DECODER_BYPASS,
			(reg | AUTO_TEMP_JUDGE_ENABLE));

	return 0;
}

int s1d13541_set_temperature(struct s1d135xx *epson, int8_t temp)
{
	assert(epson);

	epson->temp_set = temp;

	return 0;
}

int s1d13541_get_temperature(struct s1d135xx *epson, int8_t *temp)
{
	assert(epson);

	*temp = epson->temp_measured;

	return 0;
}

#define	GENERIC_TEMP_CONF_REG	0x057E
#define	DISPLAY_INT_WAVEFORM_UPDATE	0x4000

static void measured_temp(uint16_t temp_reg, uint8_t *needs_update,
			  int8_t *measured)
{
	uint16_t reg;

	epson_reg_read(DISPLAY_INT_RAW_STAT_REG, &reg);
	epson_reg_write(DISPLAY_INT_RAW_STAT_REG,
			(DISPLAY_INT_WAVEFORM_UPDATE |
			 DISPLAY_INT_TEMP_OUT_OF_RANGE));
	*needs_update = (reg & DISPLAY_INT_WAVEFORM_UPDATE) ? 1 : 0;

	epson_reg_read(temp_reg, &reg);
	*measured = (reg & 0x00ff);
}

int s1d13541_measure_temperature(struct s1d135xx *epson, u8 *needs_update)
{
	s8 temp_measured;

	assert(epson);

	switch(epson->temp_mode)
	{
	case TEMP_MODE_UNDEFINED:
	default:
		return -1;

	case TEMP_MODE_MANUAL:
		/* apply manually specified temperature */
		epson_reg_write(GENERIC_TEMP_CONF_REG,
				0xC000 | (epson->temp_set & 0x00ff));
		epson_wait_for_idle();
		measured_temp(GENERIC_TEMP_CONF_REG, needs_update,
			      &temp_measured);
		break;

	case TEMP_MODE_EXTERNAL:
		/* check temperature sensor is configured */
		epson_mode_standby(epson);
		epson_cmd_p0(RD_TEMP);
		epson_wait_for_idle();
		epson_mode_run(epson);
		measured_temp(0x0216, needs_update, &temp_measured);
		break;

	case TEMP_MODE_INTERNAL:
		/* use the internal temperature sensor */
		epson_mode_standby(epson);
		epson_cmd_p0(RD_TEMP);
		epson_wait_for_idle();
		epson_mode_run(epson);
		measured_temp(0x0576, needs_update, &temp_measured);
		break;
	}

	epson->temp_measured = temp_measured;

	printk("541: Temperature: %d\n", temp_measured);

	return 0;
}
