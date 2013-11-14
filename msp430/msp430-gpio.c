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
 * msp430-gpio.c -- MSP430 gpio pin management functions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 * The purpose of this module is to allow some dynamic control of GPIO
 * configuration. It is not intended to validate all requests completely
 * but where possible it will given the data it has available.
 *
 * It is only intended to handle 8 bit ports at the moment and can only handle
 * 1 bit at a time. It is intended to help avoid multiple use of resources
 * and to take care of all the register settings required to configure the pin.
 *
 */

#include <msp430.h>
#include <stddef.h>
#include "types.h"
#include "assert.h"
#include "msp430-gpio.h"

#define GPIO_PORT(gpio)		(((gpio) >> 8) & 0x0FF)
#define GPIO_PIN(gpio)		((gpio) & 0x0FF)

#define GPIO_CHECK_PARAMETERS	1
#define GPIO_DEBUG				0
#define	GPIO_ALLOC_FAIL_FATAL	1

// nasty token pasting stuff that allows us to parameterise the
// access to registers we need
#define PxIN(_x_)	P ##_x_ ##IN
#define PxOUT(_x_)	P ##_x_ ##OUT
#define PxDIR(_x_)	P ##_x_ ##DIR
#define PxREN(_x_)	P ##_x_ ##REN
#define PxDS(_x_)	P ##_x_ ##DS
#define PxSEL(_x_)	P ##_x_ ##SEL
#define PxIES(_x_)	P ##_x_ ##IES
#define PxIE(_x_)	P ##_x_ ##IE
#define PxIFG(_x_)	P ##_x_ ##IFG

/* could maybe store offsets if we can compute them?
 * This is a big table but its in flash. */
static const struct io_config {
	volatile u8 *in;		// if NULL then this port not present
	volatile u8 *out;
	volatile u8 *dir;
	volatile u8 *resenable;
	volatile u8 *strength;
	volatile u8 *fnselect;
	volatile u8 *edge;		// only in ports 1 & 2
	volatile u8 *intenable;	// only in ports 1 & 2
	volatile u8 *intflag;	// only in ports 1 & 2
} gpio_defs[] = {
#ifdef __MSP430_HAS_PORT1_R__
		{ &PxIN(1),	&PxOUT(1),  &PxDIR(1),  &PxREN(1),  &PxDS(1),  &PxSEL(1),  &PxIES(1),  &PxIE(1),  &PxIFG(1) },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT2_R__
		{ &PxIN(2),	&PxOUT(2),  &PxDIR(2),  &PxREN(2),  &PxDS(2),  &PxSEL(2),  &PxIES(2),  &PxIE(2),  &PxIFG(2) },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT3_R__
		{ &PxIN(3),	&PxOUT(3),  &PxDIR(3),  &PxREN(3),  &PxDS(3),  &PxSEL(3),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT4_R__
		{ &PxIN(4),	&PxOUT(4),  &PxDIR(4),  &PxREN(4),  &PxDS(4),  &PxSEL(4),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT5_R__
		{ &PxIN(5),	&PxOUT(5),  &PxDIR(5),  &PxREN(5),  &PxDS(5),  &PxSEL(5),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT6_R__
		{ &PxIN(6),	&PxOUT(6),  &PxDIR(6),  &PxREN(6),  &PxDS(6),  &PxSEL(6),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT7_R__
		{ &PxIN(7),	&PxOUT(7),  &PxDIR(7),  &PxREN(7),  &PxDS(7),  &PxSEL(7),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT8_R__
		{ &PxIN(8),	&PxOUT(8),  &PxDIR(8),  &PxREN(8),  &PxDS(8),  &PxSEL(8),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT9_R__
		{ &PxIN(9),	&PxOUT(9),  &PxDIR(9),  &PxREN(9),  &PxDS(9),  &PxSEL(9),  NULL,  NULL,  NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT10_R__
		{ &PxIN(10),&PxOUT(10), &PxDIR(10), &PxREN(10), &PxDS(10), &PxSEL(10), NULL, NULL, NULL },
#else
		{ NULL },
#endif
#ifdef __MSP430_HAS_PORT11_R__
		{ &PxIN(11),&PxOUT(11), &PxDIR(11), &PxREN(11), &PxDS(11), &PxSEL(11), NULL, NULL, NULL },
#else
		{ NULL },
#endif
};

// debug use to track allocations of gpios
static u8 alloc_map[ARRAY_SIZE(gpio_defs)];

static void gpio_mark(u16 port, u16 pinmask, int claim)
{
	if (claim)
		alloc_map[port] |= pinmask;
	else
		alloc_map[port] &= ~pinmask;
}

void gpio_mark_busy(int gpio)
{
	gpio_mark(GPIO_PORT(gpio), GPIO_PIN(gpio), true);
}

static int gpio_check(u16 port, u16 pinmask)
{
	if (port >= ARRAY_SIZE(gpio_defs))
		return -ENXIO;

	if (gpio_defs[port].in == NULL)
		return -ENXIO;

	if (alloc_map[port] & pinmask)
		return -EBUSY;

	return 0;
}

#ifndef NDEBUG
static int gpio_is_allocated(u16 port, u16 pinmask)
{
	return (alloc_map[port] & pinmask);
}
#endif

#if GPIO_DEBUG
static int gpio_pin_number(u16 pinmask)
{
	int bit = 0;
	while (pinmask >>= 1)
	{
		bit++;
	}
	return bit;
}
#endif

void gpio_init(void)
{
}

#if GPIO_CHECK_PARAMETERS
static /*inline*/ int all_set(int flags, int bits)
{
	return ((flags & bits) == bits);
}

static /*inline*/ int any_set(int flags, int bits)
{
	return ((flags & bits) != 0);
}
#endif

int gpio_request(int gpio, u16 flags)
{
	int ret;
	const struct io_config *io;
	u16 port;
	u16 pinmask;

	port    = GPIO_PORT(gpio);
	pinmask = GPIO_PIN(gpio);

#if GPIO_DEBUG
	printk("gpio_request(%02d:%02d - 0x%04x)\n", (port+1), gpio_pin_number(pinmask), flags);
#endif
	ret = gpio_check(port, pinmask);

	if ((ret == -EBUSY) && (flags & PIN_REDEFINE))
		ret = 0;

	if (ret < 0)
		goto err_alloc;

	io = &gpio_defs[port];

	/* Validate the request as best we can */
#if GPIO_CHECK_PARAMETERS
	/* must be one or the other */
	if (!any_set(flags, PIN_GPIO | PIN_SPECIAL))
		goto err_config;

	/* but cannot be a gpio AND a special purpose pin */
	if (all_set(flags, PIN_GPIO | PIN_SPECIAL))
		goto err_config;

	/* cannot be an input AND an output */
	if (all_set(flags, PIN_INPUT | PIN_OUTPUT))
		goto err_config;

	/* cannot initialise high AND low */
	if (all_set(flags, PIN_INIT_HIGH | PIN_INIT_LOW))
		goto err_config;

	/* if flags request interrupts and the port does not support them error */
	if (all_set(flags, PIN_INTERRUPT) && !io->intflag)
		goto err_config;

	/* cannot apply pull up parameters to output pins */
	if (all_set(flags, PIN_OUTPUT) && any_set(flags, PIN_PULL_UP | PIN_PULL_DOWN))
		goto err_config;

	/* cannot apply output parameters to input pins */
	if (all_set(flags, PIN_INPUT) &&
		any_set(flags, PIN_FULL_DRIVE | PIN_REDUCED_DRIVE | PIN_INIT_HIGH | PIN_INIT_LOW))
		goto err_config;
#endif

	/* Now apply the parameters to the io port */
	if (flags & PIN_GPIO)
		*io->fnselect &= ~pinmask;
	else
		*io->fnselect |= pinmask;

	if (flags & PIN_OUTPUT)
	{
		*io->dir |= pinmask;
		if (flags & PIN_INIT_HIGH)
			*io->out |= pinmask;
		else
			*io->out &= ~pinmask;

		if (flags & PIN_REDUCED_DRIVE)
			*io->strength &= ~pinmask;
		else
			*io->strength |= pinmask;
	}

	if (flags & PIN_INPUT)
	{
		*io->dir &= ~pinmask;
		*io->resenable &= ~pinmask;
		if (any_set(flags, PIN_PULL_UP | PIN_PULL_DOWN))
		{
			*io->resenable |= pinmask;
			if (flags & PIN_PULL_UP)
				*io->out |= pinmask;
			else
				*io->out &= ~pinmask;
		}

		*io->intenable &= ~pinmask;
		if (flags & PIN_INTERRUPT)
		{
			*io->intenable |= pinmask;
			if (flags & PIN_RISING_EDGE)
				*io->edge &= ~pinmask;
			else
				*io->edge |= pinmask;
		}
	}

	gpio_mark(port, pinmask, 1);

	return 0;

err_config:
	ret = -EPERM;
err_alloc:

#if GPIO_ALLOC_FAIL_FATAL
	assert(ret == 0);
#endif

	return ret;
}

void gpio_set_value(int gpio, int state)
{
	int port    = GPIO_PORT(gpio);
	int pinmask = GPIO_PIN(gpio);

	assert(gpio_is_allocated(port, pinmask));

	if (state)
		*gpio_defs[port].out |= pinmask;
	else
		*gpio_defs[port].out &= ~pinmask;
}

void gpio_set_value_hi(int gpio)
{
	int port    = GPIO_PORT(gpio);
	int pinmask = GPIO_PIN(gpio);

	assert(gpio_is_allocated(port, pinmask));

	*gpio_defs[port].out |= pinmask;
}

void gpio_set_value_lo(int gpio)
{
	int port    = GPIO_PORT(gpio);
	int pinmask = GPIO_PIN(gpio);

	assert(gpio_is_allocated(port, pinmask));

	*gpio_defs[port].out &= ~pinmask;
}

int gpio_get_value(int gpio)
{
	int port    = GPIO_PORT(gpio);
	int pinmask = GPIO_PIN(gpio);

	assert(gpio_is_allocated(port, pinmask));

	return *gpio_defs[port].in & pinmask;
}

