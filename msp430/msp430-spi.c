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
 * msp430-spi.c -- MSP430 SPI interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <pl/gpio.h>
#include <pl/interface.h>
#include <msp430.h>
#include "utils.h"
#include "assert.h"
#include "msp430-defs.h"
#include "msp430-spi.h"
#include "msp430-gpio.h"

#define CONFIG_PLAT_RUDDOCK2	1

#if CONFIG_PLAT_RUDDOCK2
#define USCI_UNIT	A
#define USCI_CHAN	0
// Pin definitions for this unit
#define	SPI_SIMO                MSP430_GPIO(3,4)
#define	SPI_SOMI                MSP430_GPIO(3,5)
#define	SPI_CLK                 MSP430_GPIO(3,0)

#endif

int msp430_spi_read_bytes(uint8_t *buff, uint8_t size);
int msp430_spi_write_bytes(uint8_t *buff, uint8_t size);
/* We only support a single SPI bus and that bus is defined at compile
 * time.
 */
int msp430_spi_init(struct pl_gpio *gpio, uint8_t spi_channel, uint16_t divisor, struct pl_interface *iface)
{
	static const struct pl_gpio_config gpios[] = {
	{ SPI_SIMO,      PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	{ SPI_SOMI,      PL_GPIO_SPECIAL | PL_GPIO_INPUT                   },
	{ SPI_CLK,       PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	};

	if (spi_channel != 0)
		return -1;

	UCxnCTL1 |= UCSWRST;					// Put state machine in reset

	if (pl_gpio_config_list(gpio, gpios, ARRAY_SIZE(gpios)))
		return -1;

	// SPI setting, MSb first, 8bit, Master Mode, 3 pin SPI, Synch Mode
	UCxnCTL0 |= (UCMST | UCSYNC | UCMSB | UCCKPH);

	UCxnCTL1 |= UCSSEL_2;					// SMCLK is selected
    UCxnBR0 = (divisor & 0x00ff);			// f_UCxCLK = 20MHz/1 = 20MHz
    UCxnBR1 = ((divisor >> 8) & 0x00ff);	//
	UCxnIE = 0x00;							// All interrupts disabled

	UCxnCTL1 &= ~UCSWRST;                  	// Release state machine from reset

	iface->read = msp430_spi_read_bytes;
	iface->write = msp430_spi_write_bytes;

	return 0;
}

void msp430_spi_close(void)
{
	UCxnCTL1 |= UCSWRST;                      // Put state machine in reset
}

int msp430_spi_read_bytes(uint8_t *buff, uint8_t size)
{
	unsigned int gie = __get_SR_register() & GIE;	// Store current GIE state

    __disable_interrupt();							// Make this operation atomic

    UCxnIFG &= ~UCRXIFG;							// Ensure RXIFG is clear

    while (size--) {
        while (!(UCxnIFG & UCTXIFG));				// Wait for TX buff empty
        UCxnTXBUF = 0x00;							// Write dummy byte to generate spi clock
        while (!(UCxnIFG & UCRXIFG));				// Wait for RX buffer (full)
        *buff++ = UCxnRXBUF;						// store the byte
    }

    __bis_SR_register(gie);                         // Restore original GIE state

    return 0;
}

int msp430_spi_write_bytes(uint8_t *buff, uint8_t size)
{
	unsigned int gie = __get_SR_register() & GIE;   // Store current GIE state

    __disable_interrupt();                          // Make this operation atomic

    // Clock the actual data transfer and send the bytes. Note that we
    // intentionally do not read out the receive buffer during frame transmission
    // in order to optimize transfer speed, however we need to take care of the
    // resulting overrun condition.
    while (size--) {
        while (!(UCxnIFG & UCTXIFG)) ;              // Wait for transmit buffer empty
        UCxnTXBUF = *buff++;                        // Write byte
    }
    while (UCxnSTAT & UCBUSY) ;                     // Wait for all TX/RX to finish

    UCxnRXBUF;                                      // Dummy read to empty RX buffer
                                                    // and clear any overrun conditions
    __bis_SR_register(gie);
    // Restore original GIE state
    return 0;
}


