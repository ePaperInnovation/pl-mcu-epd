/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2021, 2018 Plastic Logic GmbH

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
 * ust-battery.c -- Battery Management functions
 *
 * Authors:
 *   Oliver Lenz <oliver.lenz@plasticlogic.com>
 *
 */
#include <hardware/battery.h>
#include <math.h>

const uint16_t input_start = 0;
const uint16_t input_end = 1024;
const uint16_t output_start = 0;
const uint16_t output_end = 100;
double slope;

void initBattery()
{
    slope = 1.0 * (output_end - output_start) / (input_end - input_start);
}

uint16_t readBattery()
{   uint16_t input;
    //ToDo:Read Meas_Bat into Input
    uint16_t output = output_start + round(slope * (input - input_start));
    return output;
}

double round(double d)
{
    return floor(d + 0.5);
}
