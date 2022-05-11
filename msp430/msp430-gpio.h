/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013, 2014 Plastic Logic Limited

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
 * msp430-gpio.h -- MSP430 gpio pin management functions
 *
 * Authors:
 *   Nick Terry <nick.terry@plasticlogic.com>
 *   Guillaume Tucker <guillaume.tucker@plasticlogic.com>
 *
 */

#ifndef MSP430_GPIO_H
#define MSP430_GPIO_H 1

#define MSP430_GPIO(_port, _pin) (((_port) - 1) << 8 | (1 << (_pin)))

struct pl_gpio;

extern int msp430_gpio_init(struct pl_gpio *gpio);

void Setup_Flash_Write(void);
void Setup_Flash_Read(void);
void bitbang(unsigned char instr);
void bitbangL(unsigned char instr);
void Read_Status_Registers(void);
void Read_Jedec(void);
void Bump_cs(void);
void ChipErase(void);
void Read(unsigned char addr);
unsigned char CheckBusy();
unsigned char ReadStatusRegister1();
unsigned char CpuIn(void);
unsigned int SectorErase(unsigned int addr);
unsigned char CheckBusy(void);
unsigned char ReadStatusRegister1(void);
unsigned char d;


#define    so     MSP430_GPIO(5,7)
#endif /* MSP430_GPIO_H */
