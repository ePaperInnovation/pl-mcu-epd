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
 * msp430-sdcard.c -- MSP430 SD card SPI interface driver
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <pl/platform.h>
#include <pl/gpio.h>
#include "msp430.h"
#include "msp430-defs.h"
#include "types.h"
#include "assert.h"
#include "msp430-gpio.h"
#include "msp430-sdcard.h"

#define CONFIG_PLAT_RUDDOCK2	1

#if CONFIG_PLAT_RUDDOCK2
#define USCI_UNIT	B
#define USCI_CHAN	0
// Pins from MSP430 connected to the SD Card
#define	SD_CS                   MSP430_GPIO(5,5)
#define	SD_SIMO                 MSP430_GPIO(3,1)
#define	SD_SOMI                 MSP430_GPIO(3,2)
#define	SD_CLK                  MSP430_GPIO(3,3)

#else

#endif

struct platform *SDCard_plat = NULL;

void SDCard_uDelay(u16 usecs)
{
	udelay(usecs);
}

void SDCard_init(void)
{
	static const struct pl_gpio_config gpios[] = {
		{ SD_CS,   PL_GPIO_OUTPUT  | PL_GPIO_INIT_H                  },
		{ SD_SIMO, PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
		{ SD_SOMI, PL_GPIO_SPECIAL | PL_GPIO_INPUT                   },
		{ SD_CLK,  PL_GPIO_SPECIAL | PL_GPIO_OUTPUT | PL_GPIO_INIT_L },
	};

	assert(SDCard_plat != NULL);

	UCxnCTL1 |= UCSWRST;                      	// Put state machine in reset

	if (pl_gpio_config_list(&SDCard_plat->gpio, gpios, ARRAY_SIZE(gpios)))
		return;

	// SPI Master, LSb first, 3pin SPI, Sync mode
	UCxnCTL0 |= (UCMST | UCSYNC | UCCKPL | UCMSB);

	UCxnCTL1 |= UCSSEL_2;                     	// Use SMCLK, keep RESET
    UCxnBR0 = 63;                           	// Initial SPI clock must be <400kHz
    UCxnBR1 = 0;                            	// f_UCxCLK = 20MHz/63 = 317kHz
	UCxnIE = 0x00;								// disable all interrupts
	UCxnCTL1 &= ~UCSWRST;                     	// **Release state machine from reset**
}

void SDCard_fastMode (void)
{
    UCxnCTL1 |= UCSWRST;                        // Put state machine in reset
    UCxnBR0 = 1;                                // f_UCxCLK = 20MHz/1 = 20MHz
    UCxnBR1 = 0;
    UCxnCTL1 &= ~UCSWRST;                       // Release USCI state machine
}

void SDCard_readFrame (u8 *pBuffer, u16 size)
{
    uint16_t gie = __get_SR_register() & GIE;   // Store current GIE state

    __disable_interrupt();                      // Make this operation atomic

    UCxnIFG &= ~UCRXIFG;                        // Ensure RXIFG is clear

    // Clock the actual data transfer and receive the bytes
    while (size--){
        while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
        UCxnTXBUF = 0xff;                       // Write dummy byte
        while (!(UCxnIFG & UCRXIFG)) ;          // Wait for RX buffer (full)
        *pBuffer++ = UCxnRXBUF;
    }

    __bis_SR_register(gie);                     // Restore original GIE state
}

void SDCard_sendFrame (u8 *pBuffer, u16 size)
{
    uint16_t gie = __get_SR_register() & GIE;   // Store current GIE state

    __disable_interrupt();                      // Make this operation atomic

    // Clock the actual data transfer and send the bytes. Note that we
    // intentionally not read out the receive buffer during frame transmission
    // in order to optimize transfer speed, however we need to take care of the
    // resulting overrun condition.
    while (size--){
        while (!(UCxnIFG & UCTXIFG)) ;          // Wait while not ready for TX
        UCxnTXBUF = *pBuffer++;                 // Write byte
    }
    while (UCxnSTAT & UCBUSY) ;                 // Wait for all TX/RX to finish

    UCxnRXBUF;                                  // Dummy read to empty RX buffer
                                                // and clear any overrun conditions

    __bis_SR_register(gie);                     // Restore original GIE state
}

void SDCard_setCSHigh (void)
{
	SDCard_plat->gpio.set(SD_CS, 1);
}

void SDCard_setCSLow (void)
{
	SDCard_plat->gpio.set(SD_CS, 0);
}

