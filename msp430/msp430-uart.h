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
 * msp430-uart.h -- Serial UART driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef MSP430_UART_H_
#define MSP430_UART_H_

#define	BR_9600		1
#define	BR_19200	2
#define	BR_38400	3
#define	BR_57600	4
#define	BR_115200	5
#define	BR_230400	6

extern int uart_getc(void);
extern int uart_putc(int _c);
extern int uart_puts(const char *_ptr);
extern int uart_init(int baud_rate_id, char parity, int data_bits, int stop_bits);


#endif /* MSP430_UART_H_ */
