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
 * main.c -- main entry point for code.
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <epson/epson-s1d135xx.h>
#include <pl/platform.h>
#include <pl/gpio.h>
#include <pl/hwinfo.h>
#include <FatFs/ff.h>
#include <app/app.h>
#include <stdio.h>
#include "i2c-eeprom.h"
#include "assert.h"
#include "types.h"
#include "config.h"
#include "probe.h"
#include "msp430-i2c.h"
#include "msp430-gpio.h"
#include "msp430-sdcard.h"
#include "msp430-uart.h"

#define LOG_TAG "main"
#include "utils.h"

#define I2C_PSU_EEPROM_ADDR 0x50

/* Navigation buttons */
#define	SW1         MSP430_GPIO(2,0)
#define	SW2         MSP430_GPIO(2,1)
#define	SW3         MSP430_GPIO(2,2)
#define	SW4         MSP430_GPIO(2,3)
#define	SW5         MSP430_GPIO(2,4)

/* User LEDs */
#define	LED1        MSP430_GPIO(8,0)
#define	LED2        MSP430_GPIO(8,1)
#define	LED3        MSP430_GPIO(8,2)
#define	LED4        MSP430_GPIO(8,3)

/* System LEDs */
#define	ASSERT_LED  MSP430_GPIO(7,7)

/* User selection switches */
#define	SEL1        MSP430_GPIO(8,4)
#define	SEL2        MSP430_GPIO(8,5)
#define	SEL3        MSP430_GPIO(8,6)
#define	SEL4        MSP430_GPIO(8,7)

/* Version of pl-mcu-epd */
static const char VERSION[] = "v006";

/* Platform instance, to be passed to other modules */
static struct platform g_plat;

/* --- System GPIOs --- */

static const struct pl_gpio_config g_gpios[] = {
	/* User selection switches */
	{ SEL1, PL_GPIO_INPUT | PL_GPIO_PU },
	{ SEL2, PL_GPIO_INPUT | PL_GPIO_PU },
	{ SEL3, PL_GPIO_INPUT | PL_GPIO_PU },
	{ SEL4, PL_GPIO_INPUT | PL_GPIO_PU },

	/* Navigation buttons */
	{ SW1, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW2, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW3, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW4, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
	{ SW5, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },

	/* User LEDs */
	{ LED1, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ LED2, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ LED3, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ LED4, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },

	/* System LEDs */
	{ ASSERT_LED, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};

static const struct pl_system_gpio g_sys_gpio = {
	{ SEL1, SEL2, SEL3, SEL4 },
	{ SW1, SW2, SW3, SW4, SW5 },
	{ LED1, LED2, LED3, LED4 },
	ASSERT_LED,
};

/* --- HV-PMIC GPIOs --- */

#define HVSW_CTRL         MSP430_GPIO(1,2) /* VCOM switch enable */
#define PMIC_EN           MSP430_GPIO(1,1) /* HV-PMIC enable */
#define PMIC_POK          MSP430_GPIO(1,0) /* HV-PMIC power OK */

static const struct pl_gpio_config g_hvpmic_gpios[] = {
	{ HVSW_CTRL,  PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_EN,    PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ PMIC_POK,   PL_GPIO_INPUT                   },
};

static struct pl_epdpsu_gpio g_epdpsu_gpio = {
	&g_plat.gpio, PMIC_EN, HVSW_CTRL, PMIC_POK, 100,
};

/* --- Epson GPIOs --- */

/* Optional pins used in Epson SPI interface */
/* HRDY indicates controller is ready */
#define SPI_HRDY_USED     0 /* HRDY pin is used */
/* HDC required by the 524 controller, optional on others */
#define SPI_HDC_USED      1 /* HDC pin is used */

/* Basic signals to enable Epson clock and PSU */
#define EPSON_3V3_EN      MSP430_GPIO(1,7)
#define EPSON_CLK_EN      MSP430_GPIO(1,6)

/* Interface control signals */
#define	EPSON_HIRQ        MSP430_GPIO(2,6)
#define	EPSON_HDC         MSP430_GPIO(1,3)
#define	EPSON_HRDY        MSP430_GPIO(2,7)
#define EPSON_RESET       MSP430_GPIO(5,0)
#define EPSON_CS_0        MSP430_GPIO(3,6)

/* Parallel interface */
#define	EPSON_HDB0        MSP430_GPIO(4,0)
#define	EPSON_HDB1        MSP430_GPIO(4,1)
#define	EPSON_HDB2        MSP430_GPIO(4,2)
#define	EPSON_HDB3        MSP430_GPIO(4,3)
#define	EPSON_HDB4        MSP430_GPIO(4,4)
#define	EPSON_HDB5        MSP430_GPIO(4,5)
#define	EPSON_HDB6        MSP430_GPIO(4,6)
#define	EPSON_HDB7        MSP430_GPIO(4,7)
#define	EPSON_HDB8        MSP430_GPIO(6,0)
#define	EPSON_HDB9        MSP430_GPIO(6,1)
#define	EPSON_HDB10       MSP430_GPIO(6,2)
#define	EPSON_HDB11       MSP430_GPIO(6,3)
#define	EPSON_HDB12       MSP430_GPIO(6,4)
#define	EPSON_HDB13       MSP430_GPIO(6,5)
#define	EPSON_HDB14       MSP430_GPIO(6,6)
#define	EPSON_HDB15       MSP430_GPIO(6,7)

/* TFT interface extensions */
#define	EPSON_TFT_HSYNC   MSP430_GPIO(7,2)
#define	EPSON_TFT_VSYNC   MSP430_GPIO(7,3)
#define	EPSON_TFT_DE      MSP430_GPIO(7,4)
#define	EPSON_TFT_CLK     MSP430_GPIO(7,5)

static const struct pl_gpio_config g_epson_gpios[] = {
	{ EPSON_3V3_EN,  PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_CLK_EN,  PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_HIRQ,    PL_GPIO_INPUT  | PL_GPIO_PU     },
	{ EPSON_HRDY,    PL_GPIO_INPUT                   },
	{ EPSON_HDC,     PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ EPSON_RESET,   PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
	{ EPSON_CS_0,    PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
};

static const uint16_t g_epson_parallel[] = {
	EPSON_HDB0, EPSON_HDB1, EPSON_HDB2, EPSON_HDB3, EPSON_HDB4, EPSON_HDB5,
	EPSON_HDB6, EPSON_HDB7, EPSON_HDB8, EPSON_HDB9, EPSON_HDB10,
	EPSON_HDB11, EPSON_HDB12, EPSON_HDB13, EPSON_HDB14, EPSON_HDB15,
	EPSON_TFT_HSYNC, EPSON_TFT_VSYNC, EPSON_TFT_DE, EPSON_TFT_CLK,
};

static const struct s1d135xx_data g_s1d135xx_data = {
	EPSON_RESET, EPSON_CS_0, EPSON_HIRQ,
#if SPI_HRDY_USED
	EPSON_HRDY,
#else
	PL_GPIO_NONE,
#endif
#if SPI_HDC_USED
	EPSON_HDC,
#else
	PL_GPIO_NONE,
#endif
};

#define SPI_CHANNEL 0
#if CONFIG_PLAT_CUCKOO
#define SPI_DIVISOR 2
#else
#define SPI_DIVISOR 1
#endif

/* --- hardware info --- */

#if CONFIG_HW_INFO_EEPROM
static struct pl_hw_info g_pl_hw_info;
#elif CONFIG_HW_INFO_DEFAULT /* ToDo: use default as fall-back solution */
static const struct pl_hw_info g_pl_hw_info = {
	/* version */
	PL_HW_INFO_VERSION,
	/* vcom */
	{ 127, 4172, 381, 12490, 25080, -32300, 56886 },
	/* board */
#if CONFIG_PLAT_Z6
	{ "HB", 6, 3, 0, HV_PMIC_TPS65185, 0, 0, 0, I2C_MODE_HOST,
	  TEMP_SENSOR_LM75, 0, EPDC_S1D13541, 1, 1 },
#elif CONFIG_PLAT_Z7
	{ "HB", 7, 2, 0, HV_PMIC_TPS65185, 0, 0, 0, I2C_MODE_HOST,
	  TEMP_SENSOR_LM75, 0, EPDC_S1D13541, 1, 1 },
#else
# error "Sorry, no default hardware data for this platform yet."
#endif
	/* crc */
	0xFFFF,
};
#endif

/* --- main --- */

int main_init(void)
{
#if CONFIG_HW_INFO_EEPROM
	static const struct i2c_eeprom hw_eeprom = {
		&g_plat.host_i2c, I2C_PSU_EEPROM_ADDR, EEPROM_24LC014;
	};
#endif
	struct s1d135xx s1d135xx = { /* ToDo: static const? */
		&g_s1d135xx_data, &g_plat.gpio,
	};
	FATFS sdcard;
	int platform_type;
	unsigned i;

	LOG("------------------------");
	LOG("Starting pl-mcu-epd %s", VERSION);

	g_plat.sys_gpio = &g_sys_gpio;

	/* initialise GPIO interface */
	if (msp430_gpio_init(&g_plat.gpio))
		abort_msg("Failed to initialise GPIO interface");

	/* initialise system GPIOs */
	if (pl_gpio_config_list(&g_plat.gpio, g_gpios, ARRAY_SIZE(g_gpios)))
		abort_msg("Failed to initialise system GPIOs");

	/* initialize HV-PMIC GPIOs */
	if (pl_gpio_config_list(&g_plat.gpio, g_hvpmic_gpios,
				ARRAY_SIZE(g_hvpmic_gpios)))
		abort_msg("Failed to initialise HV-PMIC GPIOs");

	/* initialise Epson GPIOs */
	if (pl_gpio_config_list(&g_plat.gpio, g_epson_gpios,
				ARRAY_SIZE(g_epson_gpios)))
		abort_msg("Failed to initialise Epson GPIOs");

	/* initialise Epson parallel interface GPIOs */
	for (i = 0; i < ARRAY_SIZE(g_epson_parallel); ++i) {
		if (g_plat.gpio.config(g_epson_parallel[i],
				       PL_GPIO_OUTPUT | PL_GPIO_INIT_L)) {
			abort_msg("Failed to initialise Epson parallel GPIO");
		}
	}

	/* initialise MSP430 UART */
	if (msp430_uart_init(&g_plat.gpio, BR_115200, 'N', 8, 1))
		abort_msg("Failed to initialise UART");

	/* initialise MSP430 I2C master 0 */
	if (msp430_i2c_init(&g_plat.gpio, 0, &g_plat.host_i2c))
		abort_msg("Failed to initialise I2C master");

	/* initialise MSP430 SPI bus */
	if (spi_init(&g_plat.gpio, SPI_CHANNEL, SPI_DIVISOR))
		abort_msg("Failed to initialise SPI bus");

	/* initialise SD-card */
	SDCard_plat = &g_plat;
	f_chdrive(0);
	if (f_mount(0, &sdcard) != FR_OK)
		abort_msg("Failed to initialise SD card");

#if CONFIG_HW_INFO_EEPROM
	if (pl_hw_info_init(&pl_hw_info, &hw_eeprom))
		abort_msg("Failed to read hardware info EEPROM");
#else
	LOG("Using hard-coded hardware info");
#endif
	pl_hw_info_log(&g_pl_hw_info);

	/* initialise EPD HV-PSU */
	if (pl_epdpsu_gpio_init(&g_plat.psu, &g_epdpsu_gpio))
		abort_msg("Failed to initialise HV-PSU");

	if (probe(&g_plat, &g_pl_hw_info, &s1d135xx))
		abort_msg("Failed to probe hardware");

#if 1 /* ToDo: make plwf_load_wf work for both EEPROM and SD card */
	LOG("Sending waveform...");
	if (s1d13541_send_waveform())
		return -1;
#else
	if (plwf_load_wf(&plwf.data, pe, epson, epson_config->wf_addr))
		return -1;
#endif

	/* run the application */
	if (app_demo(&g_plat))
		abort_msg("Application failed");

	return 0;
}

/* When something fatal happens, this is called to print a message on stderr
 * and flash the "assert" LED forever.
 */
void abort_now(const char *abort_msg)
{
	if (abort_msg != NULL)
		fprintf(stderr, "%s", abort_msg);

	for (;;) {
		g_plat.gpio.set(g_plat.sys_gpio->assert_led, 1);
		mdelay(500);
		g_plat.gpio.set(g_plat.sys_gpio->assert_led, 0);
		mdelay(500);
	}
}
