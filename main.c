/*
 Plastic Logic EPD project on MSP430

 Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * Authors:
 *  Nick Terry <nick.terry@plasticlogic.com>
 *  Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

//#include <epson/epson-s1d135xx.h>
#include <ite/ite-it8951.h>
#include <pl/platform.h>
#include <pl/gpio.h>
#include <pl/interface.h>
#include <pl/hwinfo.h>
#include <pl/wflib.h>
#include <app/app.h>
#include <stdio.h>
#include "i2c-eeprom.h"
#include "vcom.h"
//#include "pmic-tps65185.h"
#include "assert.h"
#include "config.h"
#include "probe.h"
#include "msp430-i2c.h"
#include "msp430-gpio.h"
//#include "msp430-sdcard.h"
#include "msp430-uart.h"
#include "msp430-spi.h"
#include <msp430.h>

#define LOG_TAG "main"
#include "utils.h"

/* I2C addresses */
#define I2C_HWINFO_EEPROM_ADDR 0x50
#define I2C_DISPINFO_EEPROM_ADDR 0x54

/* System buttons */
#define	SW1         MSP430_GPIO(6,0)
#define	SW2         MSP430_GPIO(6,1)

/* System LEDs */
#define	ASSERT_LED  MSP430_GPIO(7,7)

/* Boster Pins */
#define	EN_5V        MSP430_GPIO(8,0)
#define	EN_3v3       MSP430_GPIO(8,2)
#define	PG_5V        MSP430_GPIO(8,1)

/* Battery Measurement */

#define BAT_MEAS       MSP430_GPIO(6,2)
#define SW_BAT_MEAS    MSP430_GPIO(6,3)

/* Logic Control Pins */
#define GPIO1        MSP430_GPIO(1,0)
#define GPIO2        MSP430_GPIO(1,1)
#define GPIO3        MSP430_GPIO(1,2)
#define GPIO4        MSP430_GPIO(1,3)
#define GPIO5        MSP430_GPIO(1,4)
#define GPIO6        MSP430_GPIO(1,5)
#define GPIO7        MSP430_GPIO(1,6)

/* Version of pl-mcu-epd */
static const char VERSION[] = "v0.1";

/* Platform instance, to be passed to other modules */
static struct pl_platform g_plat;

/* --- System GPIOs --- */

static const struct pl_gpio_config g_gpios[] = {

/* System Buttons */
{ SW1, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },
{ SW2, PL_GPIO_INPUT | PL_GPIO_INTERRUPT | PL_GPIO_INT_FALL },

/* System LEDs */
{ ASSERT_LED, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },

/* Booster Pins */
{ EN_5V, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
{ EN_3v3, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
{ PG_5V, PL_GPIO_INPUT | PL_GPIO_PU },

/* Battery Measurement */
{ BAT_MEAS, PL_GPIO_SPECIAL | PL_GPIO_INPUT},
{ SW_BAT_MEAS, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },

/* Logic Control Pins */
//{ GPIO1, PL_GPIO_SPECIAL | PL_GPIO_INPUTL},
//{ GPIO2, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
{ GPIO3, PL_GPIO_OUTPUT | PL_GPIO_INIT_L},
//{ GPIO4, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
//{ GPIO5, PL_GPIO_SPECIAL | PL_GPIO_INPUTL},
//{ GPIO6, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
//{ GPIO7, PL_GPIO_SPECIAL | PL_GPIO_INPUTL},

};

static const struct pl_system_gpio g_sys_gpio = {
        { SW1, SW2},
        { EN_5V, EN_3v3, PG_5V},
        GPIO3,
        ASSERT_LED,
        SW_BAT_MEAS,
     };

/* --- Epson GPIOs --- */

/* Optional pins used in Epson SPI interface */
/* HRDY indicates controller is ready */
#define SPI_HRDY_USED     1 /* HRDY pin is used */
/* HDC required by the 524 controller, optional on others */
#define SPI_HDC_USED      0 /* HDC pin is used */

/* Interface control signals */
#define	ITE_HRDY        MSP430_GPIO(2,7)
#define ITE_RESET       MSP430_GPIO(6,4)
#define ITE_CS_0        MSP430_GPIO(3,6)
//#define DISP_CS         MSP430_GPIO(3,6)

static const struct pl_gpio_config g_ite_gpios[] = {
        { ITE_HRDY, PL_GPIO_INPUT | PL_GPIO_PD },
        { ITE_RESET, PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
        { ITE_CS_0, PL_GPIO_OUTPUT | PL_GPIO_INIT_H },
         };


static struct it8951_data init_g_it8951_data(void)
{
    struct it8951_data g_it8951_data = { ITE_CS_0, ITE_HRDY, ITE_RESET };
    if (SPI_HRDY_USED)
        g_it8951_data.hrdy = ITE_HRDY;
    return g_it8951_data;
}

/* --- SPI bus configuration --- */

#define SPI_CHANNEL 0
#define SPI_DIVISOR 1

static struct pl_hwinfo init_hw_info_default(void)
{
    int BOARD_MAJ = 0;
    int BOARD_MIN = 0;
    if (CONFIG_HWINFO_DEFAULT)
    {
        if (global_config.board == CONFIG_PLAT_Z6_I2C
                || global_config.board == CONFIG_PLAT_Z6)
        {
            BOARD_MAJ = 6;
            BOARD_MAJ = 3;
        }
        else if (global_config.board == CONFIG_PLAT_Z7
                || global_config.board == CONFIG_PLAT_Z7_I2C)
        {
            BOARD_MAJ = 7;
            BOARD_MIN = 2;
        }
    }
    struct pl_hwinfo g_hwinfo_default;
    g_hwinfo_default.version = PL_HWINFO_VERSION;
    if (global_config.board == CONFIG_PLAT_Z6
            || global_config.board == CONFIG_PLAT_Z7)
    {
        struct pl_hw_vcom_info vcom = { 127, 4172, 381, 12490, 25080, -32300,
                                        56886 };
        g_hwinfo_default.vcom = vcom;
        struct pl_hw_board_info board = { "HB", BOARD_MAJ, BOARD_MIN, 0,
                                          HV_PMIC_TPS65185, 0, 0, 0,
                                          global_config.i2c_mode,
                                          TEMP_SENSOR_NONE, 0, EPDC_S1D13541, 1,
                                          1 };
        g_hwinfo_default.board = board;
    }
    else if (global_config.board == CONFIG_PLAT_Z6_I2C
            || global_config.board == CONFIG_PLAT_Z7_I2C)
    {
        struct pl_hw_vcom_info vcom = { 127, 4172, 381, 12490, 25080, -32300,
                                        56886 };
        g_hwinfo_default.vcom = vcom;
        struct pl_hw_board_info board = { "HB_i2c", BOARD_MAJ, BOARD_MIN, 0,
                                          HV_PMIC_TPS65185, 0, 0, 0,
                                          global_config.i2c_mode,
                                          TEMP_SENSOR_NONE, 0, EPDC_S1D13541, 1,
                                          1 };
        g_hwinfo_default.board = board;
    }
    else if (global_config.board == CONFIG_PLAT_RAVEN)
    {
        struct pl_hw_vcom_info vcom = { 63, 4586, 189, 9800, 27770, -41520,
                                        70000 };
        g_hwinfo_default.vcom = vcom;
        struct pl_hw_board_info board = { "Raven", 1, 0, 0, HV_PMIC_MAX17135, 0,
                                          0, 0, global_config.i2c_mode,
                                          TEMP_SENSOR_LM75, 0, EPDC_S1D13524, 1,
                                          1 };
        g_hwinfo_default.board = board;
    }
    else if (global_config.board == CONFIG_PLAT_FALCON)
    {
        struct pl_hw_vcom_info vcom = { 242, 1000, 968, 3833, 27770, -41520,
                                        70000 };
        g_hwinfo_default.vcom = vcom;
        struct pl_hw_board_info board = { "Falcon", 4, 1, 0, HV_PMIC_MAX17135,
                                          0, 0, 0, global_config.i2c_mode,
                                          TEMP_SENSOR_NONE, 0, EPDC_S1D13524, 1,
                                          1 };
        g_hwinfo_default.board = board;
    }
    else
    {
        LOG("Sorry, no default hardware data available for this platform.");
        exit(-1);
    }
    /* CRC16 (not used when not reading from actual EEPROM) */
    g_hwinfo_default.crc = 0xFFFF;
    return g_hwinfo_default;
}
;
/* --- waveform library and display info --- */


//static FIL g_wflib_fatfs_file;
struct pl_interface epson_spi;
struct pl_interface epson_parallel;

/* --- main --- */
char UARTdata[20];
int main_init(void)
{
   // struct pl_i2c host_i2c;
    //struct pl_i2c disp_i2c;

    struct it8951_data g_it8951_data = init_g_it8951_data();
    //if(CONFIG_HWINFO_EEPROM){
 //   const struct i2c_eeprom hw_eeprom = { &host_i2c, I2C_HWINFO_EEPROM_ADDR,
 //                                         EEPROM_24LC014, };
    //}
    struct i2c_eeprom disp_eeprom = {
    NULL,
                                      I2C_DISPINFO_EEPROM_ADDR, EEPROM_24AA256 };
    //if(CONFIG_HWINFO_EEPROM){
//    struct pl_hwinfo hwinfo_eeprom;
    //}
    struct pl_wflib_eeprom_ctx wflib_eeprom_ctx;
    struct pl_dispinfo dispinfo;
    struct vcom_cal vcom_cal;
    struct it8951 it8951 = { &g_it8951_data, &g_plat.gpio };
//    FATFS sdcard;
 //   unsigned i;

    g_plat.sys_gpio = &g_sys_gpio;

    /* initialise GPIO interface */
    if (msp430_gpio_init(&g_plat.gpio))
        abort_msg("GPIO init failed", ABORT_MSP430_GPIO_INIT);

    /* initialise system GPIOs */
    if (pl_gpio_config_list(&g_plat.gpio, g_gpios, ARRAY_SIZE(g_gpios)))
        abort_msg("System GPIO init failed", ABORT_MSP430_GPIO_INIT);

    /* initialise MSP430 UART */
    if (msp430_uart_init(&g_plat.gpio, BR_115200, 'N', 8, 1))
        abort_msg("UART init failed", ABORT_MSP430_COMMS_INIT);

    LOG("------------------------");
    LOG("Starting pl-mcu-epd %s", VERSION);

    /* initialise ITE GPIOs */
    if (pl_gpio_config_list(&g_plat.gpio, g_ite_gpios,
                            ARRAY_SIZE(g_ite_gpios)))
        abort_msg("ITE GPIO init failed", ABORT_MSP430_GPIO_INIT);

    if (global_config.interface_type == PARALLEL)
    {
        if (parallel_init(&g_plat.gpio, &epson_parallel))
            abort_msg("Parallel Interface init failed",
                      ABORT_MSP430_COMMS_INIT);
    }
    else
    {

        if (spi_init(&g_plat.gpio, SPI_CHANNEL, 2, &epson_spi))
            abort_msg("SPI init failed", ABORT_MSP430_COMMS_INIT);
        it8951.interface = &epson_spi;
    }

    //Initialise HRDY_Pin as Input
    P2DIR &= ~BIT7;
    P2OUT &= ~BIT7;
    P2REN |= BIT7;

    //ITE Restart
    g_plat.gpio.set(g_plat.sys_gpio->assert_led, 0);
    pl_gpio_set(gpio, g_it8951_data.reset, 0);
    mdelay(50);
    pl_gpio_set(gpio, g_it8951_data.reset, 1);
    mdelay(500);

    //Set 5V En Direction
    P8DIR |= BIT0;


/////////// Added by Mohamed Ahmed to read from the project or hard coded the configuration ///////////
       set_config_UST(&global_config);
     /*   if (read_config("config.txt", &global_config))
            abort_msg("Read config file failed!", ABORT_CONFIG);*/
        it8951.scrambling = global_config.scrambling;
        it8951.source_offset = global_config.source_offset;
/////////////////////////////////////////////////////////////////////////////////////////////
    struct pl_hwinfo g_hwinfo_default = init_hw_info_default();

//	/* load hardware information */
    LOG("Using default hwinfo");
    g_plat.hwinfo = &g_hwinfo_default;

    pl_hwinfo_log(g_plat.hwinfo);


    //past entry point
    /////////// Added by Mohamed Ahmed to load disp info from the project or hard coded ///////////
    if (probe_dispinfo(&dispinfo, &g_plat.epdc.wflib, &disp_eeprom, &wflib_eeprom_ctx,
                           global_config.data_source))
           // abort_msg("Failed to load dispinfo", ABORT_DISP_INFO);
        g_plat.dispinfo = &dispinfo;
        pl_dispinfo_log(&dispinfo);
    /////////////////////////////////////////////////////////////////////////////////////////////

    /* initialise EPDC */
    if (probe_epdc(&g_plat, &it8951, &vcom_cal))
        abort_msg("EPDC init failed", ABORT_EPDC_INIT);

    /* run the application */
    if (app_demo(&g_plat))
        abort_msg("Application failed", ABORT_APPLICATION);

    /*the Line below was added by Simon*/
    //app_slideshow(&g_plat, "img");

    return 0;
}

/* When something fatal happens, this is called to print a message on stderr
 * and flash the "assert" LED according to the behaviour for the error_code.
 */
void abort_now(const char *abort_msg, enum abort_error error_code)
{
    if (abort_msg != NULL)
        fprintf(stderr, "%s\r\n", abort_msg);

    /* Force LED off for case where error_code == 0 */
    g_plat.gpio.set(g_plat.sys_gpio->assert_led, 0);

    for (;;)
    {
        int i;
        mdelay(1500);
        for (i = 0; i < error_code; i++)
        {
            g_plat.gpio.set(g_plat.sys_gpio->assert_led, 1);
            mdelay(250);
            g_plat.gpio.set(g_plat.sys_gpio->assert_led, 0);
            mdelay(250);
        }
    }
}
