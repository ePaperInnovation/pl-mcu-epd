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
 * epson-s1d135xx.c -- Common Epson S1D135xx primitives
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include "epson-s1d135xx.h"
#include <pl/gpio.h>
#include <pl/endian.h>
#include <pl/types.h>
#include <stdlib.h>
#include "assert.h"

#define LOG_TAG "s1d135xx"
#include "utils.h"

/* Set to 1 to enable verbose update log messages */
#define VERBOSE_UPDATE 0

#define S1D135XX_WF_MODE(_wf) (((_wf) << 8) & 0x0F00)
#define S1D135XX_XMASK 0x01FF
#define S1D135XX_YMASK 0x03FF
#define S1D135XX_HRDY_TIMEOUT 3000
#define S1D135XX_INIT_CODE_CHECKSUM_OK (1 << 15)

enum s1d135xx_cmd {
	S1D135XX_CMD_INIT_SET         = 0x00, /* to load init code */
	S1D135XX_CMD_RUN              = 0x02,
	S1D135XX_CMD_STBY             = 0x04,
	S1D135XX_CMD_SLEEP            = 0x05,
	S1D135XX_CMD_INIT_STBY        = 0x06, /* init then standby */
	S1D135XX_CMD_READ_REG         = 0x10,
	S1D135XX_CMD_WRITE_REG        = 0x11,
	S1D135XX_CMD_BST_RD_SDR       = 0x1C,
	S1D135XX_CMD_BST_WR_SDR       = 0x1D,
	S1D135XX_CMD_BST_END_SDR      = 0x1E,
	S1D135XX_CMD_LD_IMG           = 0x20,
	S1D135XX_CMD_LD_IMG_AREA      = 0x22,
	S1D135XX_CMD_LD_IMG_END       = 0x23,
	S1D135XX_CMD_WAIT_DSPE_TRG    = 0x28,
	S1D135XX_CMD_WAIT_DSPE_FREND  = 0x29,
	S1D135XX_CMD_UPDATE_FULL      = 0x33,
	S1D135XX_CMD_UPDATE_FULL_AREA = 0x34,
	S1D135XX_CMD_EPD_GDRV_CLR     = 0x37,
};

static int get_hrdy(struct s1d135xx *p);
static int do_fill(struct s1d135xx *p, const struct pl_area *area,
		   unsigned bpp, uint8_t g);
static void send_cmd(struct s1d135xx *p, uint16_t cmd);
static void send_params(const uint16_t *params, size_t n);
static void send_param(uint16_t param);
static void set_hdc(struct s1d135xx *p, int state);
static void set_cs(struct s1d135xx *p, int state);

/* ----------------------------------------------------------------------------
 * public functions
 */

void s1d135xx_hard_reset(struct s1d135xx *p)
{
	if (p->data->reset == PL_GPIO_NONE) {
		LOG("Warning: no hard reset");
		return;
	}

	p->gpio->set(p->data->reset, 0);
	mdelay(4);
	p->gpio->set(p->data->reset, 1);
	mdelay(10);
}

int s1d135xx_soft_reset(struct s1d135xx *p)
{
	s1d135xx_write_reg(p, S1D135XX_REG_SOFTWARE_RESET, 0);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_load_init_code(struct s1d135xx *p)
{
	static const char init_code_path[] = "bin/Ecode.bin";
	FIL init_code_file;
	uint16_t checksum;
	int stat;

	if (f_open(&init_code_file, init_code_path, FA_READ) != FR_OK)
		return -1;

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_INIT_SET);
	stat = transfer_file(&init_code_file, 0, 0);
	set_cs(p, 1);
	f_close(&init_code_file);

	if (s1d135xx_wait_idle(p))
		return -1;

	if (stat) {
		LOG("Failed to transfer init code file");
		return -1;
	}

	checksum = s1d135xx_read_reg(p, S1D135XX_REG_SEQ_AUTOBOOT_CMD);

	if (!(checksum & S1D135XX_INIT_CODE_CHECKSUM_OK)) {
		LOG("Init code checksum error");
		return -1;
	}

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_INIT_STBY);
	set_cs(p, 1);
	mdelay(100);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_load_wf_lib(struct s1d135xx *p, const char *path, uint32_t addr)
{
	FIL wf_lib_file;
	uint32_t file_size;
	uint16_t params[4];

	if (f_open(&wf_lib_file, path, FA_READ) != FR_OK)
		return -1;

	file_size = f_size(&wf_lib_file);

	if (file_size & 1)
		file_size++;

	file_size /= 2;

	if (s1d135xx_wait_idle(p))
		return -1;

	params[0] = addr & 0xFFFF;
	params[1] = (addr >> 16) & 0xFFFF;
	params[2] = file_size & 0xFFFF;
	params[3] = (file_size >> 16) & 0xFFFF;
	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_BST_WR_SDR);
	send_params(params, ARRAY_SIZE(params));
	set_cs(p, 1);

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(S1D135XX_REG_HOST_MEM_PORT);
	transfer_file(&wf_lib_file, 0, 0);
	set_cs(p, 1);

	fclose(&wf_lib_file);

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_BST_END_SDR);
	set_cs(p, 1);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_init_gate_drv(struct s1d135xx *p)
{
	if (s1d135xx_set_power_state(p, PL_EPDC_RUN))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_EPD_GDRV_CLR);
	set_cs(p, 1);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_wait_dspe_trig(struct s1d135xx *p)
{
	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WAIT_DSPE_TRG);
	set_cs(p, 1);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_fill(struct s1d135xx *p, uint16_t mode, unsigned bpp,
		  const struct pl_area *a, uint8_t grey)
{
	struct pl_area full_area;
	const struct pl_area *fill_area;

	set_cs(p, 0);

	if (a != NULL) {
		const uint16_t args[] = {
			mode,
			(a->left & S1D135XX_XMASK),
			(a->top & S1D135XX_YMASK),
			(a->width & S1D135XX_XMASK),
			(a->height & S1D135XX_YMASK),
		};

		send_cmd(p, S1D135XX_CMD_LD_IMG_AREA);
		send_params(args, ARRAY_SIZE(args));
		fill_area = a;
	} else {
		send_cmd(p, S1D135XX_CMD_LD_IMG);
		send_param(mode);
		full_area.top = 0;
		full_area.left = 0;
		full_area.width = p->xres;
		full_area.height = p->yres;
		fill_area = &full_area;
	}

	set_cs(p, 1);

	return do_fill(p, fill_area, bpp, grey);
}

int s1d135xx_update(struct s1d135xx *p, int wfid, const struct pl_area *area)
{
#if VERBOSE_UPDATE
	if (area != NULL)
		LOG("update area %d (%d, %d) %dx%d", wfid,
		    area->left, area->top, area->width, area->height);
	else
		LOG("update %d", wfid);
#endif

	set_cs(p, 0);

	if (area != NULL) {
		const uint16_t params[] = {
			S1D135XX_WF_MODE(wfid),
			(area->left & S1D135XX_XMASK),
			(area->top & S1D135XX_YMASK),
			(area->width & S1D135XX_XMASK),
			(area->height & S1D135XX_YMASK),
		};

		send_cmd(p, S1D135XX_CMD_UPDATE_FULL_AREA);
		send_params(params, ARRAY_SIZE(params));
	} else {
		send_cmd(p, S1D135XX_CMD_UPDATE_FULL);
		send_param(S1D135XX_WF_MODE(wfid));
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	return s1d135xx_wait_dspe_trig(p);
}

int s1d135xx_wait_update_end(struct s1d135xx *p)
{
	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WAIT_DSPE_FREND);
	set_cs(p, 1);

	return s1d135xx_wait_idle(p);
}

int s1d135xx_wait_idle(struct s1d135xx *p)
{
	unsigned timeout = S1D135XX_HRDY_TIMEOUT;

	while (!get_hrdy(p) && --timeout)
		mdelay(1);

	if (!timeout) {
		LOG("HRDY timeout");
		return -1;
	}

	return 0;
}

int s1d135xx_set_power_state(struct s1d135xx *p,
			     enum pl_epdc_power_state state)
{
	static const uint8_t pwr_cmds[] = {
		S1D135XX_CMD_RUN, S1D135XX_CMD_STBY, S1D135XX_CMD_SLEEP,
	};

	if (state == PL_EPDC_OFF) {
		LOG("Warning: OFF mode not supported");
		return 0;
	}

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, pwr_cmds[state]);
	set_cs(p, 1);
#if 0
	mdelay(100);
#endif

	return s1d135xx_wait_idle(p);
}

void s1d135xx_cmd(struct s1d135xx *p, uint16_t cmd, const uint16_t *params,
		 size_t n)
{
	set_cs(p, 0);
	send_cmd(p, cmd);
	send_params(params, n);
	set_cs(p, 1);
}

uint16_t s1d135xx_read_reg(struct s1d135xx *p, uint16_t reg)
{
	uint16_t val;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_READ_REG);
	send_param(reg);
	spi_read_bytes((uint8_t *)&val, sizeof(uint16_t));
	spi_read_bytes((uint8_t *)&val, sizeof(uint16_t));
	set_cs(p, 1);

	return be16toh(val);
}

void s1d135xx_write_reg(struct s1d135xx *p, uint16_t reg, uint16_t val)
{
	const uint16_t params[] = { reg, val };

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_params(params, ARRAY_SIZE(params));
	set_cs(p, 1);
}

/* ----------------------------------------------------------------------------
 * private functions
 */

static int get_hrdy(struct s1d135xx *p)
{
	uint16_t status;

	if (p->data->hrdy != PL_GPIO_NONE)
		return p->gpio->get(p->data->hrdy);

	status = s1d135xx_read_reg(p, S1D135XX_REG_SYSTEM_STATUS);

	return ((status & p->hrdy_mask) == p->hrdy_result);
}

static int do_fill(struct s1d135xx *p, const struct pl_area *area,
		   unsigned bpp, uint8_t g)
{
	uint16_t val16;
	uint16_t lines;
	uint16_t pixels;

	/* Only 16-bit transfers for now... */
	assert(!(area->width % 2));

	switch (bpp) {
	case 1:
	case 2:
		LOG("Unsupported bpp");
		return -1;
	case 4:
		val16 = g & 0xF0;
		val16 |= val16 >> 4;
		val16 |= val16 << 8;
		pixels = area->width / 4;
		break;
	case 8:
		val16 = g | (g << 8);
		pixels = area->width / 2;
		break;
	default:
		LOG("Invalid bpp");
		return -1;
	}

	lines = area->height;

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_WRITE_REG);
	send_param(S1D135XX_REG_HOST_MEM_PORT);

	while (lines--) {
		uint16_t x = pixels;

		while (x--)
			send_param(val16);
	}

	set_cs(p, 1);

	if (s1d135xx_wait_idle(p))
		return -1;

	set_cs(p, 0);
	send_cmd(p, S1D135XX_CMD_LD_IMG_END);
	set_cs(p, 1);

	return s1d135xx_wait_idle(p);
}

static void send_cmd(struct s1d135xx *p, uint16_t cmd)
{
	cmd = htobe16(cmd);
	spi_write_bytes((uint8_t *)&cmd, sizeof(uint16_t));
}

static void send_params(const uint16_t *params, size_t n)
{
	size_t i;

	for (i = 0; i < n; ++i)
		send_param(params[i]);
}

static void send_param(uint16_t param)
{
	param = htobe16(param);
	spi_write_bytes((uint8_t *)&param, sizeof(uint16_t));
}

static void set_hdc(struct s1d135xx *p, int state)
{
	if (p->data->hdc != PL_GPIO_NONE)
		p->gpio->set(p->data->hdc, state);
}

static void set_cs(struct s1d135xx *p, int state)
{
	p->gpio->set(p->data->cs0, state);
}
