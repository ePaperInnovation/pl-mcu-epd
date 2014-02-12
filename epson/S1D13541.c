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
#include <string.h>
#include "types.h"
#include "assert.h"
#include "S1D13541.h"
#include "epson-cmd.h"
#include "epson-utils.h"
#include "i2c-eeprom.h"
#include "plwf.h"

#define LOG_TAG "S1D13541"
#include "utils.h"

#define EPSON_CODE_PATH	"bin/Ecode.bin"
#define WAVEFORM_PATH	"display/waveform.bin"

#define KEY 0x12345678
#define KEY_1 0x1234
#define	KEY_2 0x5678

#define GATE_POWER_BEFORE_INIT 1

/* These need changing to use our definitions */
#define TIMEOUT_MS 5000

#define REG_I2C_CLK_CONFIG         ((uint16_t)0x001A)
#define REG_CLOCK_CONFIG           ((uint16_t)0x0010)

#define BF_INIT_CODE_CHECKSUM	   ((uint16_t)(1 << 15))
#define INT_WF_CHECKSUM_ERROR	   ((uint16_t)(1 << 12))
#define INT_WF_INVALID_FORMAT	   ((uint16_t)(1 << 11))
#define INT_WF_CHECKSUM_ERROR      ((uint16_t)(1 << 12))
#define INT_WF_OVERFLOW            ((uint16_t)(1 << 13))

/* 541 register bit field value list */
#define INTERNAL_CLOCK_ENABLE      ((uint16_t)(1 << 7))
#define INTERNAL_CLOCK_DISABLE     ((uint16_t)(0 << 7))
#define INIT_CODE_CHECKSUM_ERROR   ((uint16_t)(0 << 15))
#define POWER_PASSIVE              ((uint16_t)(0 << 8))
#define POWER_ACTIVE               ((uint16_t)(1 << 8))

/* 541 constant value list */
#define PRODUCT_CODE 0x0053

static int send_init_code(void)
{
	uint16_t readval;

	if (epson_loadEpsonCode(EPSON_CODE_PATH)) {
		LOG("Failed to load instruction code from file");
		return -1;
	}

	epson_reg_read(CMD_SEQ_AUTOBOOT_CMD_REG, &readval);

	if ((readval & BF_INIT_CODE_CHECKSUM) == INIT_CODE_CHECKSUM_ERROR) {
		LOG("Init code checksum error");
		return -1;
	}

	return 0;
}

static int send_waveform(void)
{
	uint16_t readval;

	if (epson_loadEpsonWaveform(WAVEFORM_PATH, S1D13541_WF_ADDR) < 0)
		return -1;

	epson_wait_for_idle();
	epson_reg_read(DISPLAY_INT_RAW_STAT_REG, &readval);

	if (readval & INT_WF_INVALID_FORMAT)
		abort_msg("Invalid waveform format");

	if (readval & INT_WF_CHECKSUM_ERROR)
		abort_msg("Waveform checksum error");

	if (readval & INT_WF_OVERFLOW)
		abort_msg("Waveform overflow");

	return 0;
}

void s1d13541_init_display(struct s1d135xx *epson)
{
	assert(epson != NULL);

	epson_cmd_p0(UPD_INIT);
	epson_wait_for_idle();
	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();
}

void s1d13541_update_display(struct s1d135xx *epson, int waveform)
{
	assert(epson != NULL);

	epson_cmd_p1(UPD_FULL, WAVEFORM_MODE(waveform) | UPDATE_LUT(0));
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();
}

void s1d13541_update_display_area(struct s1d135xx *epson, int waveform,
				  const struct area *area)
{
	assert(epson != NULL);
	assert(area != NULL);

	epson_cmd_p5(UPD_FULL_AREA, WAVEFORM_MODE(waveform),
		     (area->left & S1D135XX_XMASK),
		     (area->top & S1D135XX_YMASK),
		     (area->width & S1D135XX_XMASK),
		     (area->height & S1D135XX_YMASK));
	epson_wait_for_idle();

	epson_cmd_p0(WAIT_DSPE_TRG);
	epson_wait_for_idle();
}

void s1d13541_wait_update_end(struct s1d135xx *epson)
{
	epson_cmd_p0(WAIT_DSPE_FREND);
	epson_wait_for_idle();
}

static void s1d13541_reset(void)
{
	epsonif_assert_reset();
	mdelay(4);
	epsonif_negate_reset();
}

static void s1d13541_init_clocks(void)
{
	epson_reg_write(REG_CLOCK_CONFIG, INTERNAL_CLOCK_ENABLE);
	msleep(10);
	epson_wait_for_idle();
	epson_reg_write(REG_I2C_CLK_CONFIG, 0x0007);
	epson_wait_for_idle();
}

int s1d13541_early_init(struct s1d135xx *epson)
{
	uint16_t product_code;

	assert(epson != NULL);

	epson_set_idle_mask(0x2000, 0x2000);

	s1d13541_reset();
	mdelay(10);
	epson_softReset();
	s1d13541_init_clocks();

	epson_reg_read(PROD_CODE_REG, &product_code);

	LOG("Product code: 0x%04x", product_code);

	if (product_code != PRODUCT_CODE) {
		LOG("invalid product code, %04X instead of %04X",
		    product_code, PRODUCT_CODE);
		return -1;
	}

	return 0;
}

int s1d13541_init(struct s1d135xx *epson)
{
	assert(epson != NULL);

	epson->keycode1 = KEY_2;
	epson->keycode2 = KEY_1;
	epson->temp_mode = TEMP_MODE_UNDEFINED;
	epson->power_mode = PWR_STATE_UNDEFINED;

	s1d13541_reset();
	epson_softReset();
	s1d13541_init_clocks();

	if (send_init_code()) {
		LOG("Oops");
		return -1;
	}

	epson_cmd_p0(INIT_SYS_STBY);
	epson->power_mode = PWR_STATE_STANDBY;
	msleep(100);
	epson_wait_for_idle();

	epson_reg_write(REG_PROTECTION_KEY_1, epson->keycode1);
	epson_reg_write(REG_PROTECTION_KEY_2, epson->keycode2);
	epson_wait_for_idle();

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
	epson_wait_for_idle();

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

	epson_reg_read(REG_LINE_DATA_LENGTH, &epson->xres);
	epson_reg_read(REG_FRAME_DATA_LENGTH, &epson->yres);

	return 0;
}

int s1d13541_send_waveform(void)
{
	return send_waveform();
}

void s1d13541_set_temperature_mode(struct s1d135xx *epson,
				   enum s1d135xx_temp_mode temp_mode)
{
	uint16_t reg;

	assert(epson != NULL);

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
		abort_msg("Invalid temperature mode");
	}

	epson_reg_write(PERIPHERAL_CONFIG_REG, reg );
	epson->temp_mode = temp_mode;

	/* Configure the controller to check for waveform update after
	 * temperature sense.  */
	epson_reg_read(REG_WAVEFORM_DECODER_BYPASS, &reg);
	epson_reg_write(REG_WAVEFORM_DECODER_BYPASS,
			(reg | AUTO_TEMP_JUDGE_ENABLE));
}

void s1d13541_set_temperature(struct s1d135xx *epson, int8_t temp)
{
	assert(epson != NULL);

	epson->temp_set = temp;
}

int8_t s1d13541_get_temperature(struct s1d135xx *epson)
{
	assert(epson != NULL);

	return epson->temp_measured;
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

void s1d13541_measure_temperature(struct s1d135xx *epson, u8 *needs_update)
{
	s8 temp_measured;

	assert(epson != NULL);
	assert(needs_update != NULL);
	assert(epson->temp_mode != TEMP_MODE_UNDEFINED);

	switch(epson->temp_mode)
	{
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

	if (temp_measured != epson->temp_measured)
		LOG("Temperature: %d", temp_measured);

	epson->temp_measured = temp_measured;
}

int s1d13541_pwrstate_sleep(struct s1d135xx *epson)
{
	uint16_t current_clk_state;
	epson_mode_sleep(epson);
	epson_wait_for_idle();
	epson_reg_read(REG_CLOCK_CONFIG, &current_clk_state);
	epson_wait_for_idle();
	epson_reg_write(REG_CLOCK_CONFIG, (current_clk_state & 0xFB7F) | INTERNAL_CLOCK_DISABLE);
	epson_wait_for_idle();
	epson_reg_write(PWR_SAVE_MODE_REG, POWER_PASSIVE);
	return 0;
}

int s1d13541_pwrstate_standby(struct s1d135xx *epson)
{
	uint16_t current_pwr_state;
	epson_reg_write(REG_CLOCK_CONFIG, INTERNAL_CLOCK_ENABLE);
	epson_wait_for_idle();
	epson_reg_read(PWR_SAVE_MODE_REG, &current_pwr_state);
	epson_wait_for_idle();
	epson_reg_write(PWR_SAVE_MODE_REG, (current_pwr_state &
			~POWER_ACTIVE) | POWER_ACTIVE);
	/* Must not check HRDY before issuing command as this won't work on a *
	 * sleep -> standby transition */
	epson_mode_standby(epson);
	epson_wait_for_idle();
	return 0;
}

int s1d13541_pwrstate_run(struct s1d135xx *epson)
{
	uint16_t current_pwr_state;
	epson_reg_write(REG_CLOCK_CONFIG, INTERNAL_CLOCK_ENABLE);
	epson_wait_for_idle();
	epson_reg_read(PWR_SAVE_MODE_REG, &current_pwr_state);
	epson_wait_for_idle();
	epson_reg_write(PWR_SAVE_MODE_REG, (current_pwr_state &
			~POWER_ACTIVE) | POWER_ACTIVE);
	epson_wait_for_idle();
	epson_mode_run(epson);
	epson_wait_for_idle();
	return 0;
}
