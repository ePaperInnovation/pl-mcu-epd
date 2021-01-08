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
 * epson-s1d13541.c -- Epson EPDC S1D13541
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "epson/epson-s1d135xx.h"
#include <pl/epdc.h>
#include <stdlib.h>
#include "assert.h"
#include "config.h"

#define LOG_TAG "s1d13541"
#include "utils.h"

#define S1D13541_PROD_CODE              0x0053
#define S1D13541_STATUS_HRDY            (1 << 13)
#define S1D13541_INTERNAL_CLOCK_ENABLE  (1 << 7)
#define S1D13541_I2C_CLOCK_DIV          7 /* 100 kHz */
#define S1D13541_PROT_KEY_1             0x5678 /* ToDo: add to s1d135xx_data */
#define S1D13541_PROT_KEY_2             0x1234
#define S1D13541_TEMP_SENSOR_CONTROL    (1 << 14)
#define S1D13541_TEMP_SENSOR_EXTERNAL   (1 << 6)
#define S1D13541_AUTO_TEMP_JUDGE_EN     (1 << 2)
#define S1D13541_GENERIC_TEMP_EN        (1 << 15)
#define S1D13541_GENERIC_TEMP_JUDGE_EN  (1 << 14)
#define S1D13541_GENERIC_TEMP_MASK      0x01FF
#define S1D13541_INT_RAW_WF_UPDATE      (1 << 14)
#define S1D13541_INT_RAW_OUT_OF_RANGE   (1 << 10)
#define S1D13541_LD_IMG_1BPP            (0 << 4)
#define S1D13541_LD_IMG_2BPP            (1 << 4)
#define S1D13541_LD_IMG_4BPP            (2 << 4)
#define S1D13541_LD_IMG_8BPP            (3 << 4)
#define S1D13541_WF_ADDR                0x00080000L

#define S1D13541_PROM_STATUS             0x0500
#define S1D13541_PROM_CTRL               0x0502
#define S1D13541_PROM_ADR_PGR_DATA 0x0504
#define S1D13541_PROM_READ_DATA          0x0506

#define S1D13541_PROM_STATUS_IDLE               0x0
#define S1D13541_PROM_STATUS_READ_BUSY          (1 << 8)
#define S1D13541_PROM_STATUS_PGM_BUSY           (1 << 9)
#define S1D13541_PROM_STATUS_ERASE_BUSY (1 << 10)
#define S1D13541_PROM_STATUS_READ_MODE          (1 << 12)
#define S1D13541_PROM_STATUS_PGM_MODE           (1 << 13)
#define S1D13541_PROM_STATUS_ERASE_ALL_MODE     (1 << 14)

#define S1D13541_PROM_READ_START                (1 << 0)
#define S1D13541_PROM_READ_STOP                 (1 << 1)
#define S1D13541_PROM_VCOM_READ                 (1 << 2)
#define S1D13541_PROM_PGM_MODE_START            (1 << 4)
#define S1D13541_PROM_PGM_OP_START                     (1 << 5)
#define S1D13541_PROM_PGM_OP_STOP               (1 << 6)
#define S1D13541_PROM_PGM_MODE_STOP             (1 << 7)
#define S1D13541_PROM_ERASE_ALL_MODE_START      (1 << 8)
#define S1D13541_PROM_ERASE_ALL_OP_START        (1 << 9)
#define S1D13541_PROM_ERASE_ALL_OP_STOP (1 << 10)
#define S1D13541_PROM_ERASE_ALL_MODE_STOP       (1 << 11)

enum s1d13541_reg {
	S1D13541_REG_CLOCK_CONFIG          = 0x0010,
	S1D13541_REG_PROT_KEY_1            = 0x042C,
	S1D13541_REG_PROT_KEY_2            = 0x042E,
	S1D13541_REG_FRAME_DATA_LENGTH     = 0x0400,
	S1D13541_REG_LINE_DATA_LENGTH      = 0x0406,
	S1D13541_REG_WF_DECODER_BYPASS     = 0x0420,
	S1D13541_REG_TEMP_SENSOR_VALUE     = 0x0576,
	S1D13541_REG_GENERIC_TEMP_CONFIG   = 0x057E,
};

enum s1d13541_cmd {
	S1D13541_CMD_RD_TEMP               = 0x12,
};
#if 1
static const struct pl_wfid s1d13541_wf_table_old[] = {
	{ 2, 1 },
	{ 3, 3 },
	{ 4, 2 },
	{ 1, 4 },
	{ 0, 0 },
	{ -1, -1 }
};
static const struct pl_wfid s1d13541_wf_table_new[] = {
	{ 2, 2 },
	{ 3, 3 },
	{ 4, 4 },
	{ 5, 5 },
	{ 6, 6 },
	{ 7, 7 },
	{ 8, 8 },
	{ 9, 9 },
	{ 10, 10 },
	{ 11, 11 },
	{ 12, 12 },
	{ 13, 13 },
	{ 14, 14 },
	{ 1, 1 },
	{ 0, 0 },
	{ -1,-1 } //-1
//	{ 2, 2 }, //2
//	{ 3, 3 }, //1
//	{ 4, 1 }, //4
//	{ 1, 1 }, //6
//	{ 0, 0 }, //0
//	{ -1,-1 } //-1
};

#endif

/* -- private functions -- */

static int s1d13541_init_clocks(struct s1d135xx *p);
static void update_temp(struct s1d135xx *p, uint16_t reg);
static int update_temp_manual(struct s1d135xx *p, int manual_temp);
static int update_temp_auto(struct s1d135xx *p, uint16_t temp_reg);
static int wait_for_ack (struct s1d135xx *p, uint16_t status, uint16_t mask);

/* -- pl_epdc interface -- */

static int s1d13541_load_wflib(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_load_wflib(p, &epdc->wflib, S1D13541_WF_ADDR);
}

static int s1d13541_set_temp_mode(struct pl_epdc *epdc,
				  enum pl_epdc_temp_mode mode)
{
	struct s1d135xx *p = epdc->data;
	uint16_t reg;

	if (mode == epdc->temp_mode)
		return 0;

	reg = s1d135xx_read_reg(p, S1D135XX_REG_PERIPH_CONFIG);
	/* ToDo: when do we set this bit back? */
	reg &= S1D13541_TEMP_SENSOR_CONTROL;

	switch (mode) {
	case PL_EPDC_TEMP_MANUAL:
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		reg &= ~S1D13541_TEMP_SENSOR_EXTERNAL;
		break;
	case PL_EPDC_TEMP_INTERNAL:
		reg |= S1D13541_TEMP_SENSOR_EXTERNAL;
		break;
	default:
		assert_fail("Invalid temperature mode");
	}

	s1d135xx_write_reg(p, S1D135XX_REG_PERIPH_CONFIG, reg);

	/* Configure the controller to automatically update the waveform table
	 * after each temperature measurement.  */
	reg = s1d135xx_read_reg(p, S1D13541_REG_WF_DECODER_BYPASS);
	reg |= S1D13541_AUTO_TEMP_JUDGE_EN;
	s1d135xx_write_reg(p, reg, S1D13541_REG_WF_DECODER_BYPASS);

	epdc->temp_mode = mode;

	return 0;
}

static int s1d13541_update_temp(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;
	int stat;

	switch (epdc->temp_mode) {
	case PL_EPDC_TEMP_MANUAL:
		stat = update_temp_manual(p, epdc->manual_temp);
		break;
	case PL_EPDC_TEMP_EXTERNAL:
		stat = update_temp_auto(p, S1D135XX_REG_I2C_TEMP_SENSOR_VALUE);
		break;
	case PL_EPDC_TEMP_INTERNAL:
		stat = update_temp_auto(p, S1D13541_REG_TEMP_SENSOR_VALUE);
		break;
	}

	if (stat)
		return -1;

	if (p->flags.needs_update) {
#if VERBOSE_TEMPERATURE
		LOG("Updating waveform table");
#endif

		if (s1d13541_load_wflib(epdc))
			return -1;
	}

	return 0;
}

static int s1d13541_fill(struct pl_epdc *epdc, const struct pl_area *area,
			 uint8_t grey)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_fill(p, S1D13541_LD_IMG_8BPP, 8, area, grey);
}

static int s1d13541_pattern_check(struct pl_epdc *epdc, uint16_t size)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_pattern_check(p, epdc->yres, epdc->xres, size, S1D13541_LD_IMG_8BPP);
}


static int s1d13541_load_image(struct pl_epdc *epdc, const char *path,
			       struct pl_area *area, int left, int top)
{
	struct s1d135xx *p = epdc->data;

	return s1d135xx_load_image(p, path, S1D13541_LD_IMG_8BPP, 8, area,
				   left, top);
}


/* -- initialisation -- */

int epson_epdc_early_init_s1d13541(struct s1d135xx *p)
{
	p->hrdy_mask = S1D13541_STATUS_HRDY;
	p->hrdy_result = S1D13541_STATUS_HRDY;
	p->measured_temp = -127;
	s1d135xx_hard_reset(p->gpio, p->data);

	if (s1d135xx_soft_reset(p))
		return -1;

	if (s1d135xx_check_prod_code(p, S1D13541_PROD_CODE))
		return -1;

	return s1d13541_init_clocks(p);
}

int epson_epdc_init_s1d13541(struct pl_epdc *epdc)
{
	struct s1d135xx *p = epdc->data;

	if (epson_epdc_early_init_s1d13541(p))
		return -1;

	if (s1d135xx_load_init_code(p)) {
		LOG("Failed to load init code");
		return -1;
	}

	if (s1d135xx_load_register_overrides(p)) {
		LOG("Failed to load register overrides");
		return -1;
	}

	// mg033 & mg034
	//s1d135xx_write_reg(p, 0x0140, 0);

	s1d135xx_write_reg(p, S1D13541_REG_PROT_KEY_1, S1D13541_PROT_KEY_1);
	s1d135xx_write_reg(p, S1D13541_REG_PROT_KEY_2, S1D13541_PROT_KEY_2);

	if (s1d135xx_wait_idle(p))
		return -1;

	if (epdc->set_power(epdc, PL_EPDC_RUN))
		return -1;

	if (s1d135xx_init_gate_drv(p))
		return -1;

	if (s1d135xx_wait_dspe_trig(p))
		return -1;

	epdc->load_wflib = s1d13541_load_wflib;
	epdc->set_temp_mode = s1d13541_set_temp_mode;
	epdc->update_temp = s1d13541_update_temp;
	epdc->fill = s1d13541_fill;
	epdc->pattern_check = s1d13541_pattern_check;
	epdc->load_image = s1d13541_load_image;
	if(global_config.waveform_version == 0){
		epdc->wf_table = s1d13541_wf_table_old;
	}else{
		epdc->wf_table = s1d13541_wf_table_new;
	}
	//epdc->wf_table = s1d13541_wf_table;
	epdc->xres = s1d135xx_read_reg(p, S1D13541_REG_LINE_DATA_LENGTH);
	epdc->yres = s1d135xx_read_reg(p, S1D13541_REG_FRAME_DATA_LENGTH);

	return epdc->set_temp_mode(epdc, PL_EPDC_TEMP_INTERNAL);
}

/* ----------------------------------------------------------------------------
 * private functions
 */

static int s1d13541_init_clocks(struct s1d135xx *p)
{
	s1d135xx_write_reg(p, S1D135XX_REG_I2C_CLOCK, S1D13541_I2C_CLOCK_DIV);
	s1d135xx_write_reg(p, S1D13541_REG_CLOCK_CONFIG,
			   S1D13541_INTERNAL_CLOCK_ENABLE);

	return s1d135xx_wait_idle(p);
}

static void update_temp(struct s1d135xx *p, uint16_t reg)
{
	uint16_t regval;

	regval = s1d135xx_read_reg(p, S1D135XX_REG_INT_RAW_STAT);
	p->flags.needs_update = (regval & S1D13541_INT_RAW_WF_UPDATE) ? 1 : 0;
	s1d135xx_write_reg(p, S1D135XX_REG_INT_RAW_STAT,
			   (S1D13541_INT_RAW_WF_UPDATE |
			    S1D13541_INT_RAW_OUT_OF_RANGE));
	regval = s1d135xx_read_reg(p, reg) & S1D135XX_TEMP_MASK;

#if VERBOSE_TEMPERATURE
	if (regval != p->measured_temp)
		LOG("Temperature: %d", regval);
#endif

	p->measured_temp = regval;
}

static int update_temp_manual(struct s1d135xx *p, int manual_temp)
{
	uint16_t regval;

	regval = (S1D13541_GENERIC_TEMP_EN |
		  S1D13541_GENERIC_TEMP_JUDGE_EN |
		  (manual_temp & S1D13541_GENERIC_TEMP_MASK));
	s1d135xx_write_reg(p, S1D13541_REG_GENERIC_TEMP_CONFIG, regval);

	if (s1d135xx_wait_idle(p))
		return -1;

	update_temp(p, S1D13541_REG_GENERIC_TEMP_CONFIG);

	return 0;
}

static int update_temp_auto(struct s1d135xx *p, uint16_t temp_reg)
{
	if (s1d135xx_set_power_state(p, PL_EPDC_STANDBY))
		return -1;

	s1d135xx_cmd(p, S1D13541_CMD_RD_TEMP, NULL, 0);

	if (s1d135xx_wait_idle(p))
		return -1;

	if (s1d135xx_set_power_state(p, PL_EPDC_RUN))
		return -1;

	update_temp(p, temp_reg);

	return 0;
}

int s1d13541_read_prom(struct s1d135xx *p, uint8_t * blob)
{
       int i = 0, j = 0;
       uint16_t data = 0;
       uint16_t addr_ = 0;

       // wait for status: idle
       if(wait_for_ack(p, S1D13541_PROM_STATUS_IDLE, 0xffff))
              return -1;

       for(i=0; i<8; i++)
       {
              for(j=0; j<2; j++)
              {
                     // set read address
                     addr_ = ((i*2+j) << 8) & 0x0f00;
                     s1d135xx_write_reg(p, S1D13541_PROM_ADR_PGR_DATA, addr_);

                     // set read operation start trigger
                     s1d135xx_write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_READ_START);

                     //wait for status: read mode start
                     if(wait_for_ack(p, S1D13541_PROM_STATUS_READ_MODE, S1D13541_PROM_STATUS_READ_MODE))
                           return -1;

                     //wait for status: read operation finished
                     if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_READ_BUSY))
                           return -1;

                     // set read operation start trigger
                     data = s1d135xx_read_reg(p, S1D13541_PROM_READ_DATA);
                     if(j)
                           blob[i] |= data & 0x0f;
                     else
                           blob[i] = data << 4 & 0xf0;

              }
       }

       // set read mode stop trigger
       s1d135xx_write_reg(p, S1D13541_PROM_CTRL, S1D13541_PROM_READ_STOP);

       //wait for status: read mode stop
       if(wait_for_ack(p, 0x0000, S1D13541_PROM_STATUS_READ_MODE))
              return -1;

       return 0;
}

int s1d13541_extract_prom_blob(uint8_t *data)
{
       int32_t bp = 0;
       int32_t sn = 0;
       int wf = 0;
       int vcom = 0;

       char c_bp[7+1] = {0,};
       char c_sn[7+1] = {0,};
       char c_wf[6+1] = {0,};

       // backplane batch ID
       bp  = ((uint32_t) data[0]) << 16;
       bp |= ((uint32_t) data[1]) << 8;
       bp |= data[2];

       // serial number
       sn  = ((uint32_t) data[3]) << 16;
       sn |= ((uint32_t) data[4]) << 8;
       sn |= data[5];

       ltoa(bp, c_bp, 10);
       ltoa(sn, c_sn, 10);
       printf("bp = %s\n", c_bp);
       printf("sn = %s\n", c_sn);

       // waveform version
       wf = data[6] >> 2 & 0x3f;
       ltoa(wf, c_wf, 10);
       printf("wf = %s\n", c_wf);

       // vcom
       vcom  = data[6] << 8 & 0x300 ;
       vcom |= data[7]      & 0xff;
       vcom *= 10; // since vcom was reduced to 10mV step size

       printf("vcom = %d\n", vcom);

       return 0;
}

static int wait_for_ack (struct s1d135xx *p, uint16_t status, uint16_t mask)
{
       unsigned long timeout = 50000;
       uint16_t v;
       while ((v = s1d135xx_read_reg(p, S1D13541_PROM_STATUS) & mask) != status){
              --timeout;
              v=v;
              if (timeout == 0){
                     LOG("PROM acknowledge timeout");
                     return -1;
              }
       }

       return 0;
}
