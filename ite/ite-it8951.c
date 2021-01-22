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

#define DATA_BUFFER_LENGTH              4096 // must be above maximum xres value for any supported display

static int get_hrdy(struct it8951 *p);
static void set_cs(struct it8951 *p, int state);
static void send_cmd(struct it8951 *p, uint16_t cmd);
static void gpio_i80_16_cmd_out(struct it8951 *p, uint16_t usCmd);
static void gpio_i80_16_data_out(struct it8951 *p, uint16_t usCmd, int size);
static uint16_t* gpio_i80_16b_data_in(struct it8951 *p, int size,
                                      uint16_t *resultBuffer);
static int it8951_writeDataBurst(struct it8951 *p, uint16_t *usData,
                                 uint16_t size);
uint16_t swap_endianess(uint16_t in);
static void set_image_buffer_base_adress(struct it8951 *p,
                                         uint32_t ulImgBufAddr);
static void do_fill(struct it8951 *p, const struct pl_area *area, unsigned bpp,
                    uint8_t g);
static int transfer_image(struct it8951 *p, FIL *f, const struct pl_area *area,
                          int left, int top, int width, int xres,
                          uint16_t scramble, uint16_t source_offset);
static void transfer_data(struct it8951 *p, const uint8_t *data, size_t n);
static int transfer_file_scrambled(struct it8951 *p, FIL *file, int xres);
static void memory_padding(uint8_t *source, uint8_t *target, int s_gl, int s_sl,
                           int t_gl, int t_sl, int o_gl, int o_sl);
static void reInitSPI(struct it8951 *p, uint8_t divisor);

uint8_t *fillBuffer;
I80IT8951DevInfo devInfo;
struct pl_area areaInfo[1];
struct pl_interface epson_spi;
uint16_t counter = 0;
uint8_t reg7[1];
uint8_t reg8[1];
uint16_t data[1], data2[1];

void it8951_load_init_code(struct it8951 *p)
{

    waitForHRDY(p);

    I80IT8951DevInfo *pBuf = (I80IT8951DevInfo*) malloc(
            sizeof(I80IT8951DevInfo));

    send_cmd(p, USDEF_I80_CMD_GET_DEV_INFO);

    reInitSPI(p, 20);


    pBuf = (I80IT8951DevInfo*) gpio_i80_16b_data_in(
            p, (sizeof(I80IT8951DevInfo) / 2), pBuf);

    p->xres = pBuf->usPanelW;
    p->yres = pBuf->usPanelH;

    devInfo = *pBuf;

    //Show Device information of IT8951
    LOG("Panel(W,H) = (%d,%d)\r\n", pBuf->usPanelW, pBuf->usPanelH);
    uint32_t imageAdress = 0xffffffff;

    reInitSPI(p, 2);


    imageAdress = 0x0;
    imageAdress = (uint32_t) pBuf->usImgBufAddrL;
    imageAdress = imageAdress | ((uint32_t) (pBuf->usImgBufAddrH) << 16);

    p->imgBufBaseAdrr = imageAdress;

    LOG("Image Buffer Address = %X%X\r\n", pBuf->usImgBufAddrH, imageAdress);

    free(pBuf);

    it8951_write_reg(p, I80CPCR, 0x0001, 1);
}

void it8951_update_Temp(struct it8951 *p, int tempMode, int temp)
{
    send_cmd(p, USDEF_I80_CMD_FORCE_SET_TEMP);

    uint16_t dataTemp[2];
    dataTemp[0] = 0x01;
    dataTemp[1] = temp;

    gpio_i80_16_data_out(p, dataTemp[0], 1);

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
    unsigned long timeout = 6000000;

    while (!(P2IN & BIT7) && timeout--)
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

    waitForHRDY(p);

    set_cs(p, 0);

    stat = p->interface->write((uint8_t*) usCmd_1, 2);

    waitForHRDY(p);

    stat = p->interface->write((uint8_t*) usCmd_2, 2);

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

    stat = p->interface->write((uint8_t*) data_, size * 2);

    set_cs(p, 1);
}

static uint16_t* gpio_i80_16b_data_in(struct it8951 *p, int size,
                                      uint16_t *iResult)
{

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

    //Read Dummy (under IT8951 SPI to I80 spec)
    waitForHRDY(p);
    //set_cs(p, 0);
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
    return iResult;
}

extern int it8951_clear_init(struct it8951 *p)
{

    return waitForHRDY(p);
}

extern int it8951_update(struct it8951 *p, int wfid, enum pl_update_mode mode,
                         struct pl_area *area)
{
    uint16_t height, width, top, left;

    if (mode == 0)
    {
        height = p->yres;
        width = p->xres;
        top = 0;
        left = 0;
    }
    else
    {
        height = area->height;
        width = area->width;
        top = area->top;
        left = area->left;
    }

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
    gpio_i80_16_data_out(p, top, 1);
    gpio_i80_16_data_out(p, left, 1);
    gpio_i80_16_data_out(p, width, 1);
    gpio_i80_16_data_out(p, height, 1);
    gpio_i80_16_data_out(p, wfid, 1);

    reInitSPI(p, 20);


    it8951_waitForDisplayReady(p);

    //uint8_t *reg7 = malloc(sizeof(uint8_t));
    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x00, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x07, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    reg7[0] = swap_endianess(gpio_i80_16b_data_in(p, 1, reg7[0]));

    LOG("PMIC Register 7 after update: 0x%x\r\n", reg7[0]);

    //uint8_t *reg8 = malloc(sizeof(uint8_t));
    send_cmd(p, IT8951_TCON_BYPASS_I2C);
    gpio_i80_16_data_out(p, 0x00, 1);
    gpio_i80_16_data_out(p, 0x68, 1);
    gpio_i80_16_data_out(p, 0x08, 1);
    gpio_i80_16_data_out(p, 0x01, 1);
    reg8[0] = swap_endianess(gpio_i80_16b_data_in(p, 1, reg8[0]));

    LOG("PMIC Register 8 after update: 0x%x\r\n", reg8[0]);

   //    free(reg7);
//    free(reg8);

    reg7[0] = 0;
    reg8[0] = 0;

    it8951_waitForDisplayReady(p);

    reInitSPI(p, 2);


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
//        send_cmd(p, USDEF_I80_CMD_POWER_CTR);
//        gpio_i80_16_data_out(p, 0x01, 1);
        waitForHRDY(p);
        data[0] = it8951_read_reg(p, 0x1e16, data);

        //FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
        waitForHRDY(p);
        data[0] |= (1 << 12);
        it8951_write_reg(p, 0x1e16, data, 1);

    }
    else if (on == 0)
    {
        // uint16_t data2 = malloc(sizeof(uint16_t));
        waitForHRDY(p);
        data2[0] = it8951_read_reg(p, 0x1e16, data2);

        send_cmd(p, USDEF_I80_CMD_POWER_CTR);
        gpio_i80_16_data_out(p, 0x01, 1);
        waitForHRDY(p);
        send_cmd(p, USDEF_I80_CMD_POWER_CTR);
        gpio_i80_16_data_out(p, 0x00, 1);
        //FLIP Bit 12 which corresponds to GPIO12/Pin 66 on ITE
        data2[0] &= ~(1 << 12);
        //FLIP Bit 11 which corresponds to GPIO11/Pin 65 on ITE to enable VCom_Switch
        data2[0] &= ~(1 << 11);
        waitForHRDY(p);
        it8951_write_reg(p, 0x1e16, data2, 1);

    }
    return 0;
}

extern int it8951_load_image(struct it8951 *p, const char *path, uint16_t mode,
                             unsigned bpp, struct pl_area *area, int left,
                             int top)
{
    struct pnm_header hdr;
    FIL img_file;
    int stat;

    struct pl_area a[1];
    a->left = 0;
    a->top = 0;
    a->width = p->xres;
    a->height = p->yres;

    if (f_open(&img_file, path, FA_READ) != FR_OK)
        return -1;

    if (pnm_read_header(&img_file, &hdr))
        return -1;

    set_image_buffer_base_adress(p, p->imgBufBaseAdrr);

    uint16_t usArg;
    //            //Setting Argument for Load image start
    usArg = (IT8951_LDIMG_L_ENDIAN << 8) | (IT8951_8BPP << 4)
            | (IT8951_ROTATE_0);
    gpio_i80_16_cmd_out(p, IT8951_TCON_LD_IMG);
    gpio_i80_16_data_out(p, usArg, 1);

    set_cs(p, 0);

    if (p->source_offset || a->width < hdr.width)
    {
        stat = transfer_file_scrambled(p, &img_file, hdr.width);
    }
    else
    {
        stat = transfer_image(p, &img_file, a, left, top, hdr.width, p->xres,
                              p->scrambling, p->source_offset);
    }

    free(a);

    set_cs(p, 1);
    f_close(&img_file);

    gpio_i80_16_cmd_out(p, IT8951_TCON_LD_IMG_END);

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
    uint16_t usData[1];
    while (it8951_read_reg(p, LUTAFSR, &usData[0]))
        ;

    return 0;
}

extern uint16_t it8951_read_reg(struct it8951 *p, uint16_t reg,
                                uint16_t *usData)
{
    gpio_i80_16_cmd_out(p, IT8951_TCON_REG_RD);
    gpio_i80_16_data_out(p, reg, 1);
    usData = gpio_i80_16b_data_in(p, 1, usData);
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
    uint16_t usData[1];
    uint16_t timeOut = 20000;
    //Check IT8951 Register LUTAFSR => NonZero ¡V Busy, 0 - Free
    while (it8951_read_reg(p, LUTAFSR, &usData[0]) != 0 && timeOut--)
        ;
    return 0;
}

void it8951_setVcom(struct it8951 *p, int vcom)
{
    //Set VCom Value
    send_cmd(p, USDEF_I80_CMD_VCOM_CTR);
    gpio_i80_16_data_out(p, 0x01, 1);
    gpio_i80_16_data_out(p, vcom, 1);
}

int it8951_fill(struct it8951 *p, const struct pl_area *area, uint8_t g)
{

    areaInfo->height = p->yres;
    areaInfo->width = p->xres;
    areaInfo->top = 0;
    areaInfo->left = 0;

    LOG("Filling Area: (%dx%d)\r\n", devInfo.usPanelW, devInfo.usPanelH);

    set_image_buffer_base_adress(p, p->imgBufBaseAdrr);

    uint16_t usArg;
    //Setting Argument for Load image start
    usArg = (IT8951_LDIMG_L_ENDIAN << 8) | (IT8951_8BPP << 4)
            | (IT8951_ROTATE_0);

    gpio_i80_16_cmd_out(p, IT8951_TCON_LD_IMG);
    gpio_i80_16_data_out(p, usArg, 1);

    do_fill(p, areaInfo, 8, g);

    free(areaInfo);

    waitForHRDY(p);

}

void do_fill(struct it8951 *p, const struct pl_area *area, unsigned bpp,
             uint8_t g)
{
    /* Only 16-bit transfers for now... */
    assert(!(area->width % 2));

    uint16_t data_[4096];

    memset(data_, g, sizeof(data_));

    int b = 0;
    for (b = 0; b < area->height; b++)
    {
        int temp = 0;
//        for (temp = 0; temp < 2; ++temp)
//        {
        it8951_writeDataBurst(p, data_, area->width / 2);
        // }
    }

    free(data_);

    LOG("Screen filled: %d\r\n", area->height);

    gpio_i80_16_cmd_out(p, IT8951_TCON_LD_IMG_END);

    return waitForHRDY(p);
}

static void reInitSPI(struct it8951 *p, uint8_t divisor)
{
    if (spi_init(p->gpio, 0, divisor, &epson_spi))
        abort_msg("SPI init failed", ABORT_MSP430_COMMS_INIT);
    p->interface = &epson_spi;
    mdelay(50);
}

int it8951_writeDataBurst(struct it8951 *p, uint16_t *usData, uint16_t size)
{
    // Prepare SPI preamble to enable ITE Write Data mode via SPI
    uint8_t preamble_[2];
    preamble_[0] = (uint8_t) 0x00;
    preamble_[1] = (uint8_t) 0x00;

    uint16_t count, swapped = 0;
    for (count = 0; count < size; count++)
    {
        swapped = (usData[count] >> 8) | (usData[count] << 8);
        usData[count] = swapped;
    }

    waitForHRDY(p);

    set_cs(p, 0);

    p->interface->write((uint8_t*) preamble_, 2);

    waitForHRDY(p);

    p->interface->write((uint8_t*) usData, size * 2);

    set_cs(p, 1);

    return waitForHRDY(p);
}

static int transfer_image(struct it8951 *p, FIL *f, const struct pl_area *area,
                          int left, int top, int width, int xres,
                          uint16_t scramble, uint16_t source_offset)
{
//LOG("%s", __func__);
    uint8_t data[DATA_BUFFER_LENGTH];
    uint8_t scrambled_data[DATA_BUFFER_LENGTH];
    uint16_t line_length = 0;
    size_t line;
    uint16_t gl = 2;
    uint16_t sl;
    scramble = 0;

    line_length = align16(xres);

    sl = line_length / 2;
    uint16_t buffer_length = max(line_length, xres);

    if (f_lseek(f, f->fptr + ((long) top * (unsigned long) width)) != FR_OK)
        return -1;

    for (line = area->height; line; --line)
    {
        size_t count;
        size_t remaining = area->width;

        /* Find the first relevant pixel (byte) on this line */
        if (f_lseek(f, f->fptr + (unsigned long) left) != FR_OK)
            return -1;

        /* Transfer data of interest in chunks */
        while (remaining)
        {
            size_t btr =
                    (remaining <= buffer_length) ? remaining : buffer_length;

            if (f_read(f, data, btr, &count) != FR_OK)
                return -1;

            if (scramble_array(data, scrambled_data, &gl, &sl, scramble))
            {
                transfer_data(p, scrambled_data, btr);
            }
            else
            {
                transfer_data(p, data, btr);
            }
            remaining -= btr;
        }

        /* Move file pointer to end of line */
        if (f_lseek(f, f->fptr + (width - (left + area->width))) != FR_OK)
            return -1;
    }

    return 0;
}

static void transfer_data(struct it8951 *p, const uint8_t *data, size_t n)
{
    uint16_t *data16 = (uint16_t*) data;
    counter = counter + 1;
    n /= 2;

    it8951_writeDataBurst(p, data16, n);
    waitForHRDY(p);
}

static int transfer_file_scrambled(struct it8951 *p, FIL *file, int xres)
{
    //LOG("%s", __func__);
    // we need to scramble the image so we need to read the file line by line
    uint8_t data[DATA_BUFFER_LENGTH];
    uint8_t scrambled_data[DATA_BUFFER_LENGTH];
    uint16_t xpad = p->source_offset;
    for (;;)
    {
        size_t count;
        uint16_t gl = 1;
        uint16_t sl = xres;
        // read one line of the image
        if (f_read(file, data, xres, &count) != FR_OK)
            return -1;

        if (!count)
            break;
        // scramble that line to up to 2 lines
        if (scramble_array(data, scrambled_data, &gl, &sl, p->scrambling))
        {
            memory_padding(scrambled_data, data, gl, sl, gl, p->xres, 0, xpad);
            transfer_data(p, data, p->xres * gl);
        }
        else
        {
            transfer_data(p, data, xres);
        }

    }

    return 0;
}

/**
 * This function pads the target (memory) with offset source and gate lines if needed.
 * If no offset is defined (o_gl=-1, o_sl=-1) the source content will be placed in the right lower corner,
 * while the left upper space is containing the offset lines.
 */
static void memory_padding(uint8_t *source, uint8_t *target, int s_gl, int s_sl,
                           int t_gl, int t_sl, int o_gl, int o_sl)
{
    int sl, gl;
    int _gl_offset = 0;
    int _sl_offset = 0;

    if (o_gl > 0)
        _gl_offset = o_gl;
    else
        _gl_offset = t_gl - s_gl;

    if (o_sl > 0)
        _sl_offset = o_sl;
    else
        _sl_offset = t_sl - s_sl;

    for (gl = 0; gl < s_gl; gl++)
        for (sl = 0; sl < s_sl; sl++)
        {
            target[(gl + _gl_offset) * t_sl + (sl + _sl_offset)] = source[gl
                    * s_sl + sl];
            source[gl * s_sl + sl] = 0xFF;
        }
}
