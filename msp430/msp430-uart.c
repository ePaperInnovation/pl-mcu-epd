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
 * msp430-uart.c -- Serial UART driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <pl/gpio.h>
#include "msp430.h"
#include "msp430-defs.h"
#include "types.h"
#include "msp430-uart.h"
#include "msp430-gpio.h"


typedef void FILE;

#define USCI_UNIT	A
#define	USCI_CHAN	1
// Pin definitions for this unit.
#define	UART_TX                 MSP430_GPIO(5,6)
#define	UART_RX                 MSP430_GPIO(5,7)

// set to 1 to have stdout, stderr sent to serial port
#define CONFIG_UART_PRINTF		0

// protect from calls before intialisation is complete.
static u8 init_done = 0;

#if CONFIG_UART_PRINTF
int fputc(int _c, register FILE *_fp);
int fputs(const char *_ptr, register FILE *_fp);
#endif

int uart_getc(void)
{
	int ret = -EAGAIN;

	if (init_done && (UCxnIFG & UCRXIFG))
	{
		ret = (UCxnRXBUF & 0x00ff);
	}
	return ret;
}

int uart_putc(int c)
{
	if (init_done)
	{
		if (c == '\n')
		{
			uart_putc('\r');
		}

		while(!(UCxnIFG & UCTXIFG));
		UCxnTXBUF = (unsigned char)c;
	}
	return((unsigned char)c);
}

int uart_puts(const char *s)
{
	unsigned int i;

	if (!init_done)
		return 1;

	for(i = 0; s[i]; i++)
	{
		uart_putc(s[i]);
	}

	return i;
}

int uart_init(struct pl_gpio *gpio, int baud_rate_id, char parity,
	      int data_bits, int stop_bits)
{
	static const struct pl_gpio_config gpios[] = {
		{ UART_TX, PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
		{ UART_RX, PL_GPIO_SPECIAL | PL_GPIO_INPUT                   },
	};

	if (pl_gpio_config_list(gpio, gpios, ARRAY_SIZE(gpios)))
		return -1;

	// hold unit in reset while configuring
	UCxnCTL1 |= UCSWRST;

	UCxnCTL0 = UCMODE_0;			// Uart Mode (No parity, LSB first, 8 data bits, 1 stop bit)
	UCxnCTL1 |= UCSSEL_2;			// SMCLK

	switch (data_bits)
	{
		case 7:
			UCxnCTL0 |= UC7BIT;
			break;
		case 8:
			UCxnCTL0 &= ~UC7BIT;
			break;
		default:
			return -EINVAL;
	}

	switch (stop_bits)
	{
		case 1:
			UCxnCTL0 &= ~UCSPB;
			break;
		case 2:
			UCxnCTL0 |= UCSPB;
			break;
		default:
			return -EINVAL;
	}

	switch (parity)
	{
		case 'N':
			UCxnCTL0 &= ~UCPEN;
			break;
		case 'E':
			UCxnCTL0 |= (UCPEN | UCPAR);
			break;
		case 'O':
			UCxnCTL0 |= UCPEN;
			break;
		default:
			return -EINVAL;
	}

	/* These registers taken from Table 34-4 and 34-5 of the
	 * MSP430 Users guide.
	 * They are dependent on a 20MHz clock
	 */
#if CPU_CLOCK_SPEED_IN_HZ != 20000000L
#error CPU Clock speed not 20MHz - baud rate calculations not valid
#endif
	switch (baud_rate_id)
	{
		case BR_9600:
			UCxnBR0 = 130;
			UCxnBR1 = 0;
			UCxnMCTL = (UCOS16 | UCBRS_0 | UCBRF_3);
			break;
		case BR_19200:
			UCxnBR0 = 65;
			UCxnBR1 = 0;
			UCxnMCTL = (UCOS16 | UCBRS_0 | UCBRF_2);
			break;
		case BR_38400:
			UCxnBR0 = 32;
			UCxnBR1 = 0;
			UCxnMCTL = (UCOS16 | UCBRS_0 | UCBRF_9);
			break;
		case BR_57600:
			UCxnBR0 = 21;
			UCxnBR1 = 0;
			UCxnMCTL = (UCOS16 | UCBRS_0 | UCBRF_11);
			break;
		case BR_115200:
			UCxnBR0 = 10;
			UCxnBR1 = 0;
			UCxnMCTL = (UCOS16 | UCBRS_0 | UCBRF_14);
			break;
		case BR_230400:
			UCxnBR0 = 5;
			UCxnBR1 = 0;
			UCxnMCTL = (UCOS16 | UCBRS_0 | UCBRF_7);
			break;
		default:
			return -EINVAL;
	}

	// release unit from reset
	UCxnCTL1 &= ~UCSWRST;

	init_done = 1;

	return 0;
}

#if CONFIG_UART_PRINTF
/* Override fputc to allow printf et al to be routed over the serial port */
int fputc(int _c, register FILE *_fp)
{
	return uart_putc(_c);
}

/* Override fputs to allow printf et al to be routed over the serial port */
int fputs(const char *_ptr, register FILE *_fp)
{
	return uart_puts(_ptr);
}

#endif
