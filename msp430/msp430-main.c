/*
 Plastic Logic EPD project on MSP430

 Copyright (C) 2013 - 2017 Plastic Logic

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
 * msp430-main.c -- MSP430 main entry point
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <msp430.h>
#include "utils.h"
#include "hal_pmm.h"

extern int main_init(void);

/* We need to disable the watchdog very early or the runtime system never gets
 * a chance to complete the initialisation of data before calling main and the
 * processor is reset.
 */
int _system_pre_init(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    return 1;
}

/***************************************************************************//**
 * @brief   Initialise the development breadboard
 * @return  None
 ******************************************************************************/
static void board_init(void)
{
    WDTCTL = WDTPW + WDTHOLD;				// Hold WDT
    SetVCore(PMMCOREV_3);					// Set VCore = 1.9V for 20MHz clock

    UCSCTL4 = SELM_4 | SELS_4 | SELA_1;	// MCLK = SMCLK = DCOCLKDIV = 20MHz /  ACLK = VLO = 10Khz
    UCSCTL3 |= SELREF_2;					// Set DCO FLL reference = REFO

    __bis_SR_register(SCG0);				// Disable the FLL control loop
    UCSCTL0 = 0x0000;						// Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_6;					// Select DCO range 30MHz operation
    UCSCTL2 = FLLD_1 + 609;					// Set DCO Multiplier for 20MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (609 + 1) * 32768 = 20MHz
                                            // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);				// Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 20 MHz / 32,768 Hz = 625000 = MCLK cycles for DCO to settle
    __delay_cycles(625000);

    // Loop until XT1,XT2 & DCO fault flag is cleared
    do
    {
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | XT1HFOFFG | DCOFFG);
        // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }
    while (SFRIFG1 & OFIFG);
}

int main(void)
{
    board_init();
    __bis_SR_register(GIE);

    return main_init();
}
