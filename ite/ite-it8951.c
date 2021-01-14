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
static void gpio_i80_16_cmd_out(struct it8951 *p, uint16_t usCmd);
static void gpio_i80_16_data_out(struct it8951 *p, uint16_t usCmd, int size);
static uint16_t* gpio_i80_16b_data_in(struct it8951 *p, int size);
static int it8951_writeDataBurst(struct it8951 *p, uint16_t *usData,
                                 uint16_t size);
uint16_t swap_endianess(uint16_t in);
static void set_image_buffer_base_adress(struct it8951 *p,
                                         uint32_t ulImgBufAddr);
static int do_fill(struct it8951 *p, const struct pl_area *area, unsigned bpp,
                   uint8_t g);

uint8_t *fillBuffer;
I80IT8951DevInfo devInfo;
struct pl_area *areaInfo;

void it8951_load_init_code(struct it8951 *p, I80IT8951DevInfo *pBuf)
{
    //it8951_set_epd_power(p, 1);
//    send_cmd(p, IT8951_TCON_BYPASS_I2C);
//    gpio_i80_16_data_out(p, 0x01, 1);
//    gpio_i80_16_data_out(p, 0x68, 1);
//    gpio_i80_16_data_out(p, 0x01, 1);
//    gpio_i80_16_data_out(p, 0x01, 1);
//    gpio_i80_16_data_out(p, 0x20, 1);

// it8951_update_Temp(p, 0, 23);
//it8951_set_epd_power(p, 1);

    send_cmd(p, USDEF_I80_CMD_GET_DEV_INFO);

    pBuf = (I80IT8951DevInfo*) gpio_i80_16b_data_in(
            p, (sizeof(I80IT8951DevInfo) / 2));

    p->xres = pBuf->usPanelW;
    p->yres = pBuf->usPanelH;

    devInfo = *pBuf;

    //Show Device information of IT8951
    LOG("Panel(W,H) = (%d,%d)\r\n", pBuf->usPanelW, pBuf->usPanelH);
    uint32_t imageAdress = 0xffffffff;

    imageAdress = 0x0;
    imageAdress = (uint32_t) pBuf->usImgBufAddrL;
    imageAdress = imageAdress | ((uint32_t) (pBuf->usImgBufAddrH) << 16);

    p->imgBufBaseAdrr = imageAdress;

    LOG("Image Buffer Address = %X%X\r\n", pBuf->usImgBufAddrH, imageAdress);

    //it8951_set_epd_power(p, 0);

    it8951_write_reg(p, I80CPCR, 0x0001, 1);
}

void it8951_update_Temp(struct it8951 *p, int tempMode, int temp)
{
    send_cmd(p, USDEF_I80_CMD_FORCE_SET_TEMP);

    uint16_t dataTemp[2];
    dataTemp[0] = 0x01;
    dataTemp[1] = temp;

    //mdelay(1);

    gpio_i80_16_data_out(p, dataTemp[0], 1);

    //mdelay(1);
    //
    gpio_i80_16_data_out(p, dataTemp[1], 1);

}

void set_image_buffer_base_adress(struct it8951 *p, uint32_t ulImgBufAddr)
{
    uint16_t usWordH = ((ulImgBufAddr >> 16) & 0x0000FFFF);
    uint16_t usWordL = (ulImgBufAddr & 0x0000FFFF);
    //Write LISAR Reg
    it8951_write_reg(p, LISAR + 2, usWordH, 1);
    it8951_write_reg(p, LISAR, usWordL, 1);
}

uint16_t swap_endianess(uint16_t in)
{

    uint8_t buff[2];

    buff[0] = (uint8_t) in;
    buff[1] = (uint8_t) (in >> 8);

    uint16_t out;

    out = buff[1];
    out = out | (buff[0] << 8);

    return out;
}

static int get_hrdy(struct it8951 *p)
{
    if (p->data->hrdy != PL_GPIO_NONE)
        return pl_gpio_get(p->gpio, p->data->hrdy);

    return 0;
}

int waitForHRDY(struct it8951 *p)
{
    unsigned long timeout = 2000000;

//    if (P2IN & BIT7)
//    {
//        //LOG("High");
//    }
//    else
//    {
//        //LOG("LOW");
//    }

    while (!(P2IN & BIT7) && --timeout)
        ;

    if (timeout == 0)
    {
        LOG("HRDY timeout");
        return -1;
    }
    return 0;
}

static void set_cs(struct it8951 *p, int state)
{
    pl_gpio_set(p->gpio, p->data->cs0, state);
}

static void send_cmd(struct it8951 *p, uint16_t cmd)
{
    gpio_i80_16_cmd_out(p, cmd);
}

static void gpio_i80_16_cmd_out(struct it8951 *p, uint16_t usCmd)
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

static void gpio_i80_16_data_out(struct it8951 *p, uint16_t usCmd, int size)
{
    int stat = 0;
    // Prepare SPI preamble to enable ITE Write Data mode via SPI
    uint8_t preamble_[2];
    preamble_[0] = (uint8_t) 0x00;
    preamble_[1] = (uint8_t) 0x00;

    // Split uint16_t Data (2byte) into its uint8 (1byte) subunits
    uint8_t data_[2];
    data_[0] = (uint8_t) (usCmd >> 8);
    data_[1] = (uint8_t) usCmd;

    waitForHRDY(p);

    set_cs(p, 0);

    stat = p->interface->write((uint8_t*) preamble_, 2);

    waitForHRDY(p);

    //udelay(10);

    stat = p->interface->write((uint8_t*) data_, size * 2);

    //udelay(10);

    set_cs(p, 1);
}

static uint16_t* gpio_i80_16b_data_in(struct it8951 *p, int size)
{
//    uint16_t usData;
    uint16_t *iResult = (uint16_t*) malloc(size * sizeof(uint16_t));

    uint16_t wPreamble = 0;
    uint16_t wDummy;
    uint32_t i;

    //set type and direction
    wPreamble = 0x1000;

    //Send Preamble before reading data
    wPreamble = MY_WORD_SWAP(wPreamble);
    waitForHRDY(p);
    set_cs(p, 0);
    p->interface->write((uint8_t*) &wPreamble, 2);

    //udelay(20);

    //Read Dummy (under IT8951 SPI to I80 spec)
    waitForHRDY(p);
    set_cs(p, 0);
    p->interface->read((uint8_t*) &wDummy, 2);

    //Read Data
    waitForHRDY(p);
    p->interface->read((uint8_t*) iResult, size * 2);
    set_cs(p, 1);

    //Convert Endian (depends on your host)
    for (i = 0; i < size; i++)
    {
        iResult[i] = MY_WORD_SWAP(iResult[i]);
    }
    // mdelay(5);
    return iResult;
}

extern int it8951_clear_init(struct it8951 *p)
{

    return waitForHRDY(p);
}

extern int it8951_update(struct it8951 *p, int wfid, enum pl_update_mode mode,
                         const struct pl_area *area)
{

    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x0B, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, 0x00, 1);

    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x0C, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, 0x00, 1);

    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x0A, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, 0x00, 1);

    send_cmd(p, USDEF_I80_CMD_DPY_AREA);
    gpio_i80_16_data_out(p, 0, 1);
    gpio_i80_16_data_out(p, 0, 1);
    gpio_i80_16_data_out(p, 1280, 1);
    gpio_i80_16_data_out(p, 960, 1);
    gpio_i80_16_data_out(p, wfid, 1);

    it8951_waitForDisplayReady(p);
    waitForHRDY(p);

    uint16_t reg7;
    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x00, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x07, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    reg7 = swap_endianess(*gpio_i80_16b_data_in(p, 1));

    LOG("PMIC Register 7 after update: 0x%x\r\n", reg7);

    waitForHRDY(p);

    uint16_t reg8;
    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x00, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x08, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    reg8 = swap_endianess(*gpio_i80_16b_data_in(p, 1));

    LOG("PMIC Register 8 after update: 0x%x\r\n", reg8);

    it8951_set_epd_power(p, 0);

    return waitForHRDY(p);
}

extern int it8951_set_power_state(struct it8951 *p,
                                  enum pl_epdc_power_state state)
{
    return waitForHRDY(p);
}

extern int it8951_set_epd_power(struct it8951 *p, int on)
{

    if (on == 1)
    {
        uint16_t data;
        data = it8951_read_reg(p, 0x1e16);

        send_cmd(p, USDEF_I80_CMD_POWER_CTR);
        gpio_i80_16_data_out(p, 0x01, 1);
        //waitForHRDY(p);
        //mdelay(250);

        //FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
        data |= (1 << 12); // switches GPIO5 of ITE (Power Up Pin) low
        //it8951_write_reg(p, 0x1e16, data, 1);

    }
    else if (on == 0)
    {

        uint16_t data2;
        data2 = it8951_read_reg(p, 0x1e16);

        send_cmd(p, USDEF_I80_CMD_POWER_CTR);
        gpio_i80_16_data_out(p, 0x00, 1);

        //FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
        data2 &= ~(1 << 12); // switches GPIO5 of ITE (Power Up Pin) low
        //FLIP Bit 11 which corresponds to GPIO11/Pin 65 on ITE to enable VCom_Switch
        //data2 &= ~(1 << 11); // switches GPIO5 of ITE (Power COM Pin) low
        it8951_write_reg(p, 0x1e16, data2, 1);
    }

    //mdelay(250);

    return 0;
}

extern int it8951_load_image(struct it8951 *p, const char *path, uint16_t mode,
                             unsigned bpp, struct pl_area *area, int left,
                             int top)
{
    return waitForHRDY(p);
}

extern int it8951_wait_idle(struct it8951 *p)
{
    return waitForHRDY(p);
}

extern void it8951_cmd(struct it8951 *p, uint16_t cmd, const uint16_t *params,
                       size_t n)
{
    gpio_i80_16_cmd_out(p, cmd);
    gpio_i80_16_data_out(p, *params, n);

}

extern int it8951_wait_update_end(struct it8951 *p)
{
    //Check IT8951 Register LUTAFSR => NonZero ¡V Busy, 0 - Free
    while (it8951_read_reg(p, LUTAFSR))
        ;
    return 0;
}

extern uint16_t it8951_read_reg(struct it8951 *p, uint16_t reg)
{
    uint16_t *usData;
    gpio_i80_16_cmd_out(p, IT8951_TCON_REG_RD);
    gpio_i80_16_data_out(p, reg, 1);
    usData = gpio_i80_16b_data_in(p, 1);
    return *usData;

}

extern void it8951_write_reg(struct it8951 *p, uint16_t reg, uint16_t val,
                             int size)
{
    gpio_i80_16_cmd_out(p, IT8951_TCON_REG_WR);
    gpio_i80_16_data_out(p, reg, 1);
    gpio_i80_16_data_out(p, val, size);

}

int it8951_waitForDisplayReady(struct it8951 *p)
{
    //Check IT8951 Register LUTAFSR => NonZero ¡V Busy, 0 - Free
    while (it8951_read_reg(p, LUTAFSR))
        ;
    return 0;
}

void it8951_setVcom(struct it8951 *p, int vcom)
{
    //Set VCom Value
    send_cmd(p, USDEF_I80_CMD_VCOM_CTR);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, vcom, 1);
    // waitForHRDY(p);
    //mdelay(250);
}

int it8951_fill(struct it8951 *p, const struct pl_area *area, uint8_t g)
{

    areaInfo->height = p->yres;
    areaInfo->width = p->xres;
    areaInfo->top = 0;
    areaInfo->left = 0;

    //fillBuffer = malloc(devInfo.usPanelW * devInfo.usPanelH);
    LOG("Filling Area: (%dx%d)\r\n", devInfo.usPanelW, devInfo.usPanelH);

//    uint32_t imageAdress = 0xffffffff;
//
//    imageAdress = 0x0;
//    imageAdress = (uint32_t) devInfo.usImgBufAddrL;
//    imageAdress = imageAdress | ((uint32_t) (devInfo.usImgBufAddrH) << 16);

    set_image_buffer_base_adress(p, 0x00122A70);

    uint16_t usArg;
    //Setting Argument for Load image start
    usArg = (IT8951_LDIMG_L_ENDIAN << 8) | (IT8951_8BPP << 4)
            | (IT8951_ROTATE_0);

    gpio_i80_16_cmd_out(p, IT8951_TCON_LD_IMG);
    gpio_i80_16_data_out(p, usArg, 1);

    return do_fill(p, areaInfo, 8, g);

    //memset(fillBuffer, g, devInfo.usPanelW * devInfo.usPanelH);

}

int do_fill(struct it8951 *p, const struct pl_area *area, unsigned bpp,
            uint8_t g)
{
    uint16_t val16;
    uint16_t lines;
    uint16_t pixels;

    /* Only 16-bit transfers for now... */
    assert(!(area->width % 2));

    switch (bpp)
    {
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
        assert_fail("Invalid bpp");
    }

    lines = area->height;

    waitForHRDY(p);

    uint16_t greyValue = val16;
    // Split uint16_t Data (2byte) into its uint8 (1byte) subunits
    uint8_t data_[1280];

    memset(data_, g, sizeof(data_));

    int b, j = 0;
    for (b = 0; b < 960; b++)
    {
        it8951_writeDataBurst(p, data_, 640);
//        for (j = 0; j < 1280; j++)
//        {
//            gpio_i80_16_data_out(p, 0xffff, 1);
//        }

    }

    LOG("Screen filled: %d\r\n", area->height);

    gpio_i80_16_cmd_out(p, IT8951_TCON_LD_IMG_END);

    return waitForHRDY(p);
}

int it8951_writeDataBurst(struct it8951 *p, uint16_t *usData, uint16_t size)
{
    // Prepare SPI preamble to enable ITE Write Data mode via SPI
    uint8_t preamble_[2];
    preamble_[0] = (uint8_t) 0x00;
    preamble_[1] = (uint8_t) 0x00;

//    uint8_t data_[2];
//       data_[0] = (uint8_t) (usCmd >> 8);
//       data_[1] = (uint8_t) usCmd;

    //swap16_array(&usData, size);

    waitForHRDY(p);

    set_cs(p, 0);

    p->interface->write((uint8_t*) preamble_, 2);

    waitForHRDY(p);

    p->interface->write((uint8_t*) usData, size * 2);

    set_cs(p, 1);

    return waitForHRDY(p);
}
