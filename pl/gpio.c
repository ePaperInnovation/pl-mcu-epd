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
 * gpio.c -- GPIO interface abstraction layer
 *
 * Authors:
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#include <pl/gpio.h>
#include "assert.h"
#include <msp430.h>

#define LOG_TAG "gpio"
#include "utils.h"

int pl_gpio_config_list(struct pl_gpio *gpio,
                        const struct pl_gpio_config *config, size_t n)
{
    size_t i;

    assert(gpio != NULL);
    assert(gpio->config != NULL);
    assert(config != NULL);

    for (i = 0; i < n; ++i)
    {
        const struct pl_gpio_config *c = &config[i];

        if (gpio->config(c->gpio, c->flags))
            return -1;
    }




    return 0;
}

#if PL_GPIO_DEBUG
void pl_gpio_log_flags(uint16_t flags)
{
LOG("flags: 0x%04X %s%s%s%s%s%s%s%s%s%s%s%s",
        flags,
        (flags & PL_GPIO_INPUT) ? "IN " : "",
        (flags & PL_GPIO_PU) ? "PU " : "",
        (flags & PL_GPIO_PD) ? "PD " : "",
        (flags & PL_GPIO_INTERRUPT) ? "INT " : "",
        (flags & PL_GPIO_INT_RISE) ? "RISE " : "",
        (flags & PL_GPIO_INT_FALL) ? "FALL " : "",
        (flags & PL_GPIO_OUTPUT) ? "OUT " : "",
        (flags & PL_GPIO_INIT_H) ? "H " : "",
        (flags & PL_GPIO_INIT_L) ? "L " : "",
        (flags & PL_GPIO_DRIVE_FULL) ? "FULL " : "",
        (flags & PL_GPIO_DRIVE_REDUCED) ? "REDUCED " : "",
        (flags & PL_GPIO_SPECIAL) ? "SPECIAL " : "");
}
#endif

int pl_gpio_check_flags(uint16_t flags)
{
    /* must be either input, output or special */
    if (!(flags & (PL_GPIO_INPUT | PL_GPIO_OUTPUT | PL_GPIO_SPECIAL)))
    {
        LOG("GPIO must be either input, output or special");
        return -1;
    }

    /* can't be both output and input */
    if ((flags & PL_GPIO_INPUT) && (flags & PL_GPIO_OUTPUT))
    {
        LOG("GPIO input and output flags conflict");
        return -1;
    }

    if (flags & PL_GPIO_INPUT)
    {
        /* can't use these flags with inputs */
        if (flags
                & (PL_GPIO_INIT_H | PL_GPIO_INIT_L | PL_GPIO_DRIVE_FULL
                        | PL_GPIO_DRIVE_REDUCED))
        {
            LOG("Incompatible flags with input");
            return -1;
        }

        /* can't be both pull-high and pull-down */
        if ((flags & PL_GPIO_PU) && (flags & PL_GPIO_PD))
        {
            LOG("Input pull up and down flags conflict");
            return -1;
        }

        if (flags & PL_GPIO_INTERRUPT)
        {
            /* must be either rising or falling edge */
            if (!(flags & (PL_GPIO_INT_RISE | PL_GPIO_INT_FALL)))
            {
                LOG("Interrupt edge must be defined");
                return -1;
            }

            /* but not both */
            if ((flags & PL_GPIO_INT_RISE) && (flags & PL_GPIO_INT_FALL))
            {
                LOG("Interrupt edge flags conflict");
                return -1;
            }
        }
    }
    else if (flags & PL_GPIO_OUTPUT)
    {
        /* can't use these flags with outputs */
        if (flags
                & (PL_GPIO_PU | PL_GPIO_PD | PL_GPIO_INTERRUPT
                        | PL_GPIO_INT_RISE | PL_GPIO_INT_FALL))
        {
            LOG("Incompatible flags with output");
            return -1;
        }

        /* initial state must be high or low */
        if (!(flags & (PL_GPIO_INIT_H | PL_GPIO_INIT_L)))
        {
            LOG("Initial output state must be defined");
            return -1;
        }

        /* but not both */
        if ((flags & PL_GPIO_INIT_H) && (flags & PL_GPIO_INIT_L))
        {
            LOG("Initial output state flags conflict");
            return -1;
        }

        /* can't be full and reduced drive strength */
        if ((flags & PL_GPIO_DRIVE_FULL) && (flags & PL_GPIO_DRIVE_REDUCED))
        {
            LOG("Output drive strength flags conflict");
            return -1;
        }
    }

    return 0;
}
