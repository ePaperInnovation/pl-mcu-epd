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
 *   Oliver Lenz <oliver.lenz@plasticlogic.com>
 *
 */

#include "ite-it8951.h"
#include <pl/gpio.h>
#include <pl/wflib.h>
#include <pl/endian.h>
#include <pl/types.h>
#include <stdlib.h>
#include <string.h>
#include <pl/interface.h>
#include "assert.h"
#include <msp430.h>

/* until the i/o operations are abstracted */
#include "pnm-utils.h"

#define LOG_TAG "it8951"
#include "utils.h"

#include <app/parser.h>

static int get_hrdy(struct it8951 *p);
static void set_cs(struct it8951 *p, int state);
static void send_cmd(struct it8951 *p, uint16_t cmd);
static void gpio_i80_16_cmd_out(struct it8951 *p, TWord usCmd);
static void gpio_i80_16_data_out(struct it8951 *p, TWord usCmd, int size);
static TWord* gpio_i80_16b_data_in(struct it8951 *p, int size);
TWord swap_endianess(TWord in);

void it8951_load_init_code(struct it8951 *p, void *pBuf)
{
    I80IT8951DevInfo *pstDevInfo;
    I80IT8951DevInfo *pBuf_ = (I80IT8951DevInfo*) pBuf;

    send_cmd(p, USDEF_I80_CMD_GET_DEV_INFO);

    __delay_cycles(25);

    pstDevInfo = (I80IT8951DevInfo*) gpio_i80_16b_data_in(
            p, (sizeof(I80IT8951DevInfo) / 2));

    //Show Device information of IT8951
    printf("Panel(W,H) = (%d,%d)\n", pstDevInfo->usPanelW,
           pstDevInfo->usPanelH);
    uint32_t imageAdress;
    imageAdress = (uint32_t) pstDevInfo->usImgBufAddrL
            | ((uint32_t) pstDevInfo->usImgBufAddrH << 16);
    printf("Image Buffer Address = %X\r\n", imageAdress);

    //Show Firmware and LUT Version
    printf("FW Version = %s\r\n", swap_endianess(*pstDevInfo->usFWVersion));
    printf("LUT Version = %s\r\n", swap_endianess(*pstDevInfo->usLUTVersion));

    it8951_write_reg(p, I80CPCR, 0x0001, 1);
//    gpio_i80_16_cmd_out(p, USDEF_I80_CMD_GET_DEV_INFO);
//    //Wait for ready ?

    send_cmd(p, USDEF_I80_CMD_FORCE_SET_TEMP);

    TWord dataTemp[2];
    dataTemp[0] = 0x0001;
    dataTemp[1] = 0x0023;

    __delay_cycles(25);

    gpio_i80_16_data_out(p, dataTemp[0], 1);

    __delay_cycles(25);

    gpio_i80_16_data_out(p, dataTemp[1], 1);

    //it8951_cmd(p, USDEF_I80_CMD_GET_DEV_INFO, NULL, 2);

}

TWord swap_endianess(TWord in)
{

    uint8_t buff[2];

    buff[0] = (uint8_t) in;
    buff[1] = (uint8_t) (in >> 8);

    TWord out;

    out = buff[1];
    out = out | (buff[0] << 8);

    return out;
}

static int get_hrdy(struct it8951 *p)
{
//    if (p->data->hrdy != PL_GPIO_NONE)
//    {
//
//        int i = 0;
//        int test = 15;
//
//        while (i++ < 10000)
//        {
//            test = pl_gpio_get(p->gpio, p->data->hrdy);
//            if (pl_gpio_get(p->gpio, p->data->hrdy) == 1)
//            {
//                return 0;
//            }
//            uint16_t status;
//
//        }
//    }
    if (p->data->hrdy != PL_GPIO_NONE)
        return pl_gpio_get(p->gpio, p->data->hrdy);
}

void waitForHRDY(struct it8951 *p)
{
    unsigned long timeout = 200000;

    if (P2IN & BIT7)
    {
        //LOG("High");
    }
    else
    {
        //LOG("LOW");
    }

    while (!(P2IN & BIT7) && --timeout)
        ;
    //LOG("HRDY state: %i", get_hrdy(p));;

    if (timeout == 0)
    {
        LOG("HRDY timeout");
        return -1;
    }
}

static void set_cs(struct it8951 *p, int state)
{
    pl_gpio_set(p->gpio, p->data->cs0, state);

    __delay_cycles(500);
}

static void send_cmd(struct it8951 *p, uint16_t cmd)
{
    gpio_i80_16_cmd_out(p, cmd);
}

static void gpio_i80_16_cmd_out(struct it8951 *p, TWord usCmd)
{
    int stat = 0;
    uint8_t usCmd_1[2];
    usCmd_1[0] = (uint8_t) 0x60;
    usCmd_1[1] = (uint8_t) 0x00;

    uint8_t usCmd_2[2];
    usCmd_2[0] = (uint8_t) (usCmd >> 8);
    usCmd_2[1] = (uint8_t) usCmd;

    //swap_endianess(usCmd_2);
//    uint8_t usCmd_2[2];
//    usCmd_2[0] = (uint8_t) 0x03;
//    usCmd_2[1] = (uint8_t) 0x02;

    waitForHRDY(p);

    set_cs(p, 0);

    stat = p->interface->write((uint8_t*) usCmd_1, 2);
    //LOG("Write status %i",stat);

    waitForHRDY(p);

    stat = p->interface->write((uint8_t*) usCmd_2, 2);
    //LOG("Write status %i",stat);

    set_cs(p, 1);
}

static void gpio_i80_16_data_out(struct it8951 *p, TWord usCmd, int size)
{
    int stat = 0;
    // Prepare SPI preamble to enable ITE Write Data mode via SPI
    uint8_t preamble_[2];
    preamble_[0] = (uint8_t) 0x00;
    preamble_[1] = (uint8_t) 0x00;

    // Split TWord Data (2byte) into its uint8 (1byte) subunits
    uint8_t data_[2];
    data_[0] = (uint8_t) (usCmd >> 8);
    data_[1] = (uint8_t) usCmd;

    waitForHRDY(p);

    set_cs(p, 0);
    stat = p->interface->write((uint8_t*) preamble_, 2);

    //__delay_cycles(2000);
    waitForHRDY(p);

    stat = p->interface->write((uint8_t*) data_, size * 2);

    set_cs(p, 1);
}

static TWord* gpio_i80_16b_data_in(struct it8951 *p, int size)
{
    TWord usData;
    TWord *iResult = (TWord*) malloc(size * sizeof(TWord));

    TByte preamble_[2];
    preamble_[0] = (TByte) 0x10;
    preamble_[1] = (TByte) 0x00;

    int stat = 0;

    waitForHRDY(p);

    set_cs(p, 0);

    stat = p->interface->write((uint8_t*) preamble_, 2);

    int i = 0;
    for (i = 0; i < (size + 1); i++)
    {
        // throw away first read, as this only contains rubbish (as documented by ITE)
        if (i == 0)
        {
            waitForHRDY(p);
            p->interface->read(&usData, 2);
            //set_cs(p, 1);
            waitForHRDY(p);
        }
        //next read will give the value that you want to receive
        else
        {
//            __delay_cycles(25);
            p->interface->read(&usData, 2);
            iResult[i - 1] = swap_endianess(usData);
            waitForHRDY(p);
            // __delay_cycles(1);

        }

    }
    set_cs(p, 1);
    return iResult;
}

extern int it8951_clear_init(struct it8951 *p)
{
}

extern int it8951_update(struct it8951 *p, int wfid, enum pl_update_mode mode,
                         const struct pl_area *area)
{
}

extern int it8951_set_power_state(struct it8951 *p,
                                  enum pl_epdc_power_state state)
{
}

extern int it8951_set_epd_power(struct it8951 *p, int on)
{
}

extern int it8951_load_image(struct it8951 *p, const char *path, uint16_t mode,
                             unsigned bpp, struct pl_area *area, int left,
                             int top)
{
}

extern int it8951_wait_idle(struct it8951 *p)
{
}

extern void it8951_cmd(struct it8951 *p, uint16_t cmd, const TWord *params,
                       size_t n)
{
    gpio_i80_16_cmd_out(p, cmd);
    gpio_i80_16_data_out(p, *params, n);

}

extern int it8951_wait_update_end(struct it8951 *p)
{

}

extern uint16_t it8951_read_reg(struct it8951 *p, uint16_t reg)
{

}

extern void it8951_write_reg(struct it8951 *p, uint16_t reg, uint16_t val,
                             int size)
{
    gpio_i80_16_cmd_out(p, IT8951_TCON_REG_WR);
    //__delay_cycles(2000);
    gpio_i80_16_data_out(p, reg, 1);
    //__delay_cycles(2000);
    gpio_i80_16_data_out(p, val, size);

}
