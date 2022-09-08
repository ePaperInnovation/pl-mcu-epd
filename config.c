/*
 * Copyright (C) 2017 Plastic Logic
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
/*
 * config.c
 *
 * Author:
 *   Robert Pohlink <robert.pohlink@plasticlogic.com>
 *
 */
#include "config.h"
#include "app/parser.h"
#include <pl/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG_TAG "config"
#include "utils.h"


struct config global_config;
static const char SEP[] = " ";

enum endianess get_endianess(char *str)
{
    if (strcmp(str, "CONFIG_BIG_ENDIAN") == 0)
        return CONFIG_BIG_ENDIAN;
    else
        return CONFIG_LITTLE_ENDIAN;
}

enum config_interface_type get_interface_type(char *str)
{
    if (strcmp(str, "PARALLEL") == 0)
        return PARALLEL;
    else
        return SPI;
}

enum config_platform_board get_board(char *str)
{
    if (strcmp(str, "CONFIG_PLAT_Z6") == 0)
        return CONFIG_PLAT_Z6;
    else if (strcmp(str, "CONFIG_PLAT_Z7") == 0)
        return CONFIG_PLAT_Z7;
    else if (strcmp(str, "CONFIG_PLAT_Z6_I2C") == 0)
        return CONFIG_PLAT_Z6_I2C;
    else if (strcmp(str, "CONFIG_PLAT_Z7_I2C") == 0)
        return CONFIG_PLAT_Z7_I2C;
    else if (strcmp(str, "CONFIG_PLAT_FALCON") == 0)
        return CONFIG_PLAT_FALCON;
    else
        return CONFIG_PLAT_RAVEN;
}
;

;

enum i2c_mode_id get_i2c_mode(char *str)
{
    if (strcmp(str, "I2C_MODE_HOST") == 0)
        return I2C_MODE_HOST;
    else if (strcmp(str, "I2C_MODE_DISP") == 0)
        return I2C_MODE_DISP;
    else if (strcmp(str, "I2C_MODE_S1D13524") == 0)
        return I2C_MODE_S1D13524;
    else if (strcmp(str, "I2C_MODE_SC18IS6XX") == 0)
        return I2C_MODE_SC18IS6XX;
    else
        return I2C_MODE_NONE;
}
;
int set_config_UST(struct config *config){
    config->i2c_mode = I2C_MODE_HOST;
    config->board = CONFIG_PLAT_FALCON;
    config->data_source = CONFIG_DISP_DATA_UST;
    config->endianess = CONFIG_LITTLE_ENDIAN;
    config->interface_type = SPI;
    config->scrambling = 104;
    int *timings[] = { &config->pmic_timings[0],
                       &config->pmic_timings[1],
                       &config->pmic_timings[2],
                       &config->pmic_timings[3],
                       &config->pmic_timings[4],
                       &config->pmic_timings[5],
                       &config->pmic_timings[6],
                       &config->pmic_timings[7],
                       NULL };
    strncpy(config->config_display_type, "S468_T1.1",
                        9);
}
