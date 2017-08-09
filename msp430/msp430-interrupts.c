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
 * msp430-interrupts.c -- Interrupt handling code for MSP430
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include <msp430.h>
#include "msp430-gpio.h"
#include <stdint.h>

uint8_t port2_int_summary = 0;

/***************************************************************************//**
 * @brief   Port 2 interrupt service routine
 * @return  None
 ******************************************************************************/
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{

	unsigned int gie = __get_SR_register();  //Store current GIE state

	__disable_interrupt();

    switch(__even_in_range(P2IV,16))
    {
    	case P2IV_P2IFG0:					// Bit 0
    		port2_int_summary |= BIT0;
    		break;
    	case P2IV_P2IFG1:					// Bit 1
    		port2_int_summary |= BIT1;
    		break;
    	case P2IV_P2IFG2:					// Bit 2
    		port2_int_summary |= BIT2;
    		break;
    	case P2IV_P2IFG3:					// Bit 3
    		port2_int_summary |= BIT3;
    		break;
    	case P2IV_P2IFG4:					// Bit 4
    		port2_int_summary |= BIT4;
    		break;
    	case P2IV_P2IFG5:					// Bit 5
    		port2_int_summary |= BIT5;
    		break;
    	case P2IV_P2IFG6:					// Bit 6
    		port2_int_summary |= BIT6;
    		break;
    	case P2IV_P2IFG7:					// Bit 7
    		port2_int_summary |= BIT7;
    		break;
    	case P2IV_NONE:						// No interrupt
    	default:
    		break;
    }
    __bis_SR_register(gie);					// Restore original GIE state
}

/* These vectors are used in the code so cannot be declared here */
#if 0
#pragma vector=PORT2_VECTOR
#pragma vector=TIMER0_A1_VECTOR
#pragma vector=RTC_VECTOR
#endif
/* Initialize unused ISR vectors with a trap function */
#pragma vector=USCI_B3_VECTOR
#pragma vector=USCI_A3_VECTOR
#pragma vector=USCI_B1_VECTOR
#pragma vector=USCI_A1_VECTOR
#pragma vector=PORT1_VECTOR
#pragma vector=TIMER1_A1_VECTOR
#pragma vector=TIMER1_A0_VECTOR
#pragma vector=DMA_VECTOR
#pragma vector=USCI_B2_VECTOR
#pragma vector=USCI_A2_VECTOR
#pragma vector=TIMER0_A0_VECTOR
#pragma vector=ADC12_VECTOR
#pragma vector=USCI_B0_VECTOR
#pragma vector=USCI_A0_VECTOR
#pragma vector=WDT_VECTOR
#pragma vector=TIMER0_B1_VECTOR
#pragma vector=TIMER0_B0_VECTOR
#pragma vector=UNMI_VECTOR
#pragma vector=SYSNMI_VECTOR
__interrupt void ISR_trap(void)
{
  // the following will cause an access violation which results in a PUC reset
  WDTCTL = 0;
}


