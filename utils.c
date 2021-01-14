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
 * utils.c -- random homeless functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *          Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/types.h>
#include <pl/endian.h>
#include <stdlib.h>
#include <stdio.h>
#include "FatFs/ff.h"
#include "msp430-gpio.h"
#include "pnm-utils.h"
#include "assert.h"
#define LOG_TAG "utils"
#include "utils.h"

void swap32(void *x)
{
    uint8_t *b = x;
    uint8_t tmp;

    tmp = b[0];
    b[0] = b[3];
    b[3] = tmp;
    tmp = b[1];
    b[1] = b[2];
    b[2] = tmp;
}

void swap32_array(int32_t **x, uint16_t n)
{
    while (n--)
        swap32(*x++);
}

void swap16(void *x)
{
    uint8_t *b = x;
    uint8_t tmp;

    tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
}

void swap16_array(uint16_t **x, uint16_t n)
{
    while (n--)
        swap16(*x++);
}

int is_file_present(const char *path)
{
    FIL f;

    if (f_open(&f, path, FA_READ) != FR_OK)
        return 0;

    f_close(&f);

    return 1;
}

int join_path(char *path, size_t n, const char *dir, const char *file)
{
    return (snprintf(path, n, "%s/%s", dir, file) >= n) ? -1 : 0;
}

int open_image(const char *dir, const char *file, FIL *f,
               struct pnm_header *hdr)
{
    char path[MAX_PATH_LEN];

    if (snprintf(path, MAX_PATH_LEN, "%s/%s", dir, file) >= MAX_PATH_LEN)
    {
        LOG("File path is too long, max=%d", MAX_PATH_LEN);
        return -1;
    }

    if (f_open(f, path, FA_READ) != FR_OK)
    {
        LOG("Failed to open image file");
        return -1;
    }

    if (pnm_read_header(f, hdr) < 0)
    {
        LOG("Failed to parse PGM header");
        return -1;
    }

    return 0;
}

/* ----------------------------------------------------------------------------
 * Debug utilies
 */

/* Defined in main.c */
extern void abort_now(const char *abort_msg, enum abort_error error_code);

static void do_abort_msg(const char *file, unsigned line, const char *error_str,
                         const char *message, enum abort_error error_code)
{
    /* Following conversion of line to a string is a workaround
     * for a problem with fprintf(stderr, "%u", line) that only
     * occurs when NOT debugging and prevents further code execution
     * (possibly a heap size issue?)
     */
    char temp[16];
    sprintf(temp, "%u", line);
    fprintf(stderr, "%s, line %s: %s\n", file, temp, message);

    abort_now(error_str, error_code);
}

void do_abort_msg_assert(const char *file, unsigned line, const char *message)
{
    do_abort_msg(file, line, "Assertion failed\n", message, ABORT_ASSERT);
}

void do_abort_msg_error(const char *file, unsigned line, const char *message,
                        enum abort_error error_code)
{
    do_abort_msg(file, line, "Fatal error\n", message, error_code);
}

void dump_hex(const void *data, uint16_t len)
{
    static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
                                  '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    char s[] = "[XXXX] XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX";
    char *cur;
    uint16_t i;

    if (!len)
        return;

    for (i = 0, cur = s; i < len; ++i)
    {
        const uint8_t byte = ((const uint8_t*) data)[i];

        if (!(i & 0xF))
        {
            uint16_t addr = i;
            uint16_t j;

            if (i)
                puts(s);

            cur = s + 4;

            for (j = 4; j; --j)
            {
                *cur-- = hex[addr & 0xF];
                addr >>= 4;
            }

            cur = s + 7;
        }

        *cur++ = hex[byte >> 4];
        *cur++ = hex[byte & 0xF];
        ++cur;
    }

    i %= 16;

    if (i)
    {
        cur = s + 6 + (i * 3);
        *cur++ = '\n';
        *cur++ = '\0';
    }

    puts(s);
}

uint16_t align8(uint16_t value)
{
    return (((value + 7) / 8) * 8);
}

uint16_t align16(uint16_t value)
{
    return (((value + 15) / 16) * 16);
}

static uint16_t calcPixelIndex(uint16_t gl, uint16_t sl, uint16_t slCount);

uint16_t scramble_array(uint8_t *source, uint8_t *target, uint16_t *glCount,
                        uint16_t *slCount, uint16_t scramblingMode)
{

    uint16_t sl, gl;
    uint16_t targetIdx;
    uint16_t sourceIdx;
    uint16_t _glCount = *glCount;
    uint16_t _slCount = *slCount;
    uint16_t __glCount;
    uint16_t __slCount;

    if (scramblingMode == 0)
    {
        // no need to scramble image data, just copy
        return 0;
    }
    else
    {
        // need to scramble image data based on scrambling mode
        for (gl = 0; gl < _glCount; gl++)
        {
            for (sl = 0; sl < _slCount; sl++)
            {
                __glCount = _glCount;
                __slCount = _slCount;

                targetIdx = calcScrambledIndex(scramblingMode, gl, sl,
                                               &__glCount, &__slCount);
                sourceIdx = calcPixelIndex(gl, sl, _slCount);
                target[targetIdx] = source[sourceIdx];
                source[sourceIdx] = 0xFF;
                //LOG("sourceIdx: %i, targetIdx: %i", sourceIdx, targetIdx);
            }
        }
        *glCount = __glCount;
        *slCount = __slCount;
        return 1;
    }
}

uint16_t calcScrambledIndex(uint16_t scramblingMode, uint16_t gl, uint16_t sl,
                            uint16_t *glCount, uint16_t *slCount)
{
    // set starting values
    uint16_t newGlIdx = gl;
    uint16_t newSlIdx = sl;
    uint16_t _glCount = *glCount;
    uint16_t _slCount = *slCount;

    // source line scrambling for half nbr of gate lines and double nbr of source lines
    // scrambling between the scrambling resolution
    if (scramblingMode & SCRAMBLING_SOURCE_SCRAMBLE_MASK)
    {
        _glCount = _glCount / 2;
        _slCount = _slCount * 2;

        if (scramblingMode & SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_MASK)
        {
            newSlIdx = (newGlIdx % 2) ? newSlIdx * 2 : (newSlIdx * 2 + 1);
            newGlIdx = newGlIdx / 2;
        }
        else
        {
            newSlIdx = (newGlIdx % 2) ? (newSlIdx * 2 + 1) : newSlIdx * 2;
            newGlIdx = newGlIdx / 2;
        }
    }
    // gate line scrambling for half nbr of source lines and double nbr of gate lines
    else if (scramblingMode & SCRAMBLING_GATE_SCRAMBLE_MASK)
    {

        _glCount = _glCount * 2;
        _slCount = _slCount / 2;

        if (scramblingMode & SCRAMBLING_SCRAMBLE_FIRST_ODD_LINE_MASK)
        {
            // scrambling between image resolution and scrambling resolution
            // move every even source line to the next gate line
            // by bisect the source line index
            newGlIdx = (newGlIdx * 2) + (newSlIdx + 1) % 2;
            newSlIdx = (newSlIdx / 2);
        }
        else
        {
            // scrambling between image resolution and scrambling resolution
            // move every odd source line to the next gate line
            // by bisect the source line index
            newGlIdx = (newGlIdx * 2) + newSlIdx % 2;
            newSlIdx = (newSlIdx / 2);
        }
    }

    // check for difference in source interlaced setting
    if (scramblingMode & SCRAMBLING_SOURCE_INTERLACED_MASK)
    {
        if (scramblingMode & SCRAMBLING_SOURCE_INTERLACED_FIRST_ODD_LINE_MASK)
        {
            newSlIdx =
                    ((newSlIdx + 1) % 2) ?
                            ((newSlIdx / 2) + _slCount / 2) : (newSlIdx / 2);
        }
        else
        {
            newSlIdx =
                    ((newSlIdx) % 2) ?
                            ((newSlIdx / 2) + _slCount / 2) : (newSlIdx / 2);
        }
    }

    // mirrors the first image half
    if (scramblingMode & SCRAMBLING_SOURCE_MIRROR_LH_MASK)
    {
        if (newSlIdx < _slCount / 2)
        {
            newSlIdx = (_slCount / 2 - 1) - newSlIdx;
        }
    }

    // mirrors the second image half
    if (scramblingMode & SCRAMBLING_SOURCE_MIRROR_RH_MASK)
    {
        if (newSlIdx >= _slCount / 2)
        {
            newSlIdx = (_slCount - 1) - (newSlIdx - (_slCount / 2));
        }
    }

    // check for difference in source direction setting
    if (scramblingMode & SCRAMBLING_SOURCE_DIRECTION_MASK)
    {
        newSlIdx = _slCount - newSlIdx - 1;
    }

    // check for difference in source direction setting
    if (scramblingMode & SCRAMBLING_SOURCE_START_MASK)
    {
        newSlIdx = (newSlIdx + _slCount / 2) % _slCount;
    }

    // check for difference in gate direction setting
    if (scramblingMode & SCRAMBLING_GATE_DIRECTION_MASK)
    {
        newGlIdx = _glCount - newGlIdx - 1;
    }

    *glCount = _glCount;
    *slCount = _slCount;

    return calcPixelIndex(newGlIdx, newSlIdx, _slCount);
}

static uint16_t calcPixelIndex(uint16_t gl, uint16_t sl, uint16_t slCount)
{
    return gl * slCount + sl;
}
