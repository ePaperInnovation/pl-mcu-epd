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
 * msp430-defs.h -- MSP430 useful definitions
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#ifndef MSP430_DEFS_H_
#define MSP430_DEFS_H_

#define PREEXPAND(x, _x_, _n_, y)	EXPAND(x, _x_, _n_, y)
#define EXPAND(x, _x_, _n_, y)	x ##_x_ ##_n_ ##y

#define	UCxnCTL0	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, CTL0)
#define	UCxnCTL1	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, CTL1)
#define UCxnBR0		PREEXPAND(UC, USCI_UNIT, USCI_CHAN, BR0)
#define	UCxnBR1		PREEXPAND(UC, USCI_UNIT, USCI_CHAN, BR1)
#define	UCxnIE		PREEXPAND(UC, USCI_UNIT, USCI_CHAN, IE)
#define	UCxnIFG		PREEXPAND(UC, USCI_UNIT, USCI_CHAN, IFG)
#define	UCxnTXBUF	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, TXBUF)
#define	UCxnRXBUF	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, RXBUF)
#define	UCxnSTAT	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, STAT)

#define	UCxnI2COA	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, I2COA)
#define	UCxnI2CSA  	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, I2CSA)

#define	UCxnMCTL  	PREEXPAND(UC, USCI_UNIT, USCI_CHAN, MCTL)

#endif /* MSP430_DEFS_H_ */
