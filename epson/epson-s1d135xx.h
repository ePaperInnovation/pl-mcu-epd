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
 * epson-s1d135xx.h -- Common Epson S1D135xx primitives
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef INCLUDE_EPSON_S1D135XX_H
#define INCLUDE_EPSON_S1D135XX_H

#include <pl/epdc.h>
#include <pl/interface.h>
#include <stdint.h>
#include <stdlib.h>

struct pl_gpio;
struct pl_wflib;

/* Set to 1 to enable verbose temperature log messages */
#define VERBOSE_TEMPERATURE                  0
#define S1D135XX_TEMP_MASK                   0x00FF

enum s1d135xx_reg {
	S1D135XX_REG_REV_CODE              = 0x0002,
	S1D135XX_REG_SOFTWARE_RESET        = 0x0008,
	S1D135XX_REG_SYSTEM_STATUS         = 0x000A,
	S1D135XX_REG_I2C_CLOCK             = 0x001A,
	S1D135XX_REG_PERIPH_CONFIG         = 0x0020,
	S1D135XX_REG_HOST_MEM_PORT         = 0x0154,
	S1D135XX_REG_I2C_TEMP_SENSOR_VALUE = 0x0216,
	S1D135XX_REG_I2C_STATUS            = 0x0218,
	S1D135XX_REG_PWR_CTRL              = 0x0230,
	S1D135XX_REG_SEQ_AUTOBOOT_CMD      = 0x02A8,
	S1D135XX_REG_DISPLAY_BUSY          = 0x0338,
	S1D135XX_REG_INT_RAW_STAT          = 0x033A,
};

enum s1d135xx_rot_mode {
	S1D135XX_ROT_MODE_0   = 0,
	S1D135XX_ROT_MODE_90  = 1,
	S1D135XX_ROT_MODE_180 = 2,
	S1D135XX_ROT_MODE_270 = 3,
};

struct s1d135xx_data {
	unsigned reset;
	unsigned cs0;
	unsigned hirq;
	unsigned hrdy;
	unsigned hdc;
	unsigned clk_en;
	unsigned vcc_en;
};

struct s1d135xx {
	const struct s1d135xx_data *data;
	struct pl_gpio *gpio;
	struct pl_interface *interface;
	uint16_t scrambling;
	uint16_t source_offset;
	uint16_t hrdy_mask;
	uint16_t hrdy_result;
	int measured_temp;
	unsigned xres;
	unsigned yres;
	struct {
		uint8_t needs_update:1;
	} flags;
};

extern void s1d135xx_hard_reset(struct pl_gpio *gpio,
				const struct s1d135xx_data *data);
extern int s1d135xx_soft_reset(struct s1d135xx *p);
extern int s1d135xx_check_prod_code(struct s1d135xx *p, uint16_t code);
extern int s1d135xx_load_init_code(struct s1d135xx *p);
extern int s1d135xx_load_wflib(struct s1d135xx *p, struct pl_wflib *wflib,
			       uint32_t addr);
extern int s1d135xx_init_gate_drv(struct s1d135xx *p);
extern int s1d135xx_wait_dspe_trig(struct s1d135xx *p);
extern int s1d135xx_clear_init(struct s1d135xx *p);
extern int s1d135xx_fill(struct s1d135xx *p, uint16_t mode, unsigned bpp,
			 const struct pl_area *a, uint8_t grey);
extern int s1d135xx_pattern_check(struct s1d135xx *p, uint16_t height,
			uint16_t width, uint16_t checker_size, uint16_t mode);
extern int s1d135xx_load_image(struct s1d135xx *p, const char *path,
			       uint16_t mode, struct pl_area *area, int left, int top);
extern int s1d135xx_update(struct s1d135xx *p, int wfid,
				enum pl_update_mode mode,
				const struct pl_area *area);
extern int s1d135xx_wait_update_end(struct s1d135xx *p);
extern int s1d135xx_wait_idle(struct s1d135xx *p);
extern int s1d135xx_set_power_state(struct s1d135xx *p,
				    enum pl_epdc_power_state state);
extern int s1d135xx_set_epd_power(struct s1d135xx *p, int on);
extern void s1d135xx_cmd(struct s1d135xx *p, uint16_t cmd,
			 const uint16_t *params, size_t n);
extern uint16_t s1d135xx_read_reg(struct s1d135xx *p, uint16_t reg);
extern void s1d135xx_write_reg(struct s1d135xx *p, uint16_t reg, uint16_t val);
extern int s1d135xx_load_register_overrides(struct s1d135xx *p);

extern int s1d13541_extract_prom_blob(uint8_t *data);
extern int s1d13541_read_prom(struct s1d135xx *p, uint8_t * blob);

#endif /* INCLUDE_EPSON_S1D135XX_H */
