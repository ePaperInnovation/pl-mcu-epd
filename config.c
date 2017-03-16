/*
 * Copyright (C) 2016 Plastic Logic
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

#include "config.h"
#include "app/parser.h"
#include <pl/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG_TAG "config"
#include "utils.h"

/*
 * config.txt on the root sdcard folder
*/
struct config global_config;
static const char SEP[] = " ";

enum endianess get_endianess(char* str){
	if(strcmp(str, "CONFIG_BIG_ENDIAN") == 0)
		return CONFIG_BIG_ENDIAN;
	else
		return CONFIG_LITTLE_ENDIAN;
}

enum config_platform_board get_board(char* str){
	if(strcmp(str, "CONFIG_PLAT_Z6") == 0)
		return CONFIG_PLAT_Z6;
	else if(strcmp(str, "CONFIG_PLAT_Z7") == 0)
		return CONFIG_PLAT_Z7;
	else
		return CONFIG_PLAT_RAVEN;
};

enum config_data_source get_data_source(char*str){
	if(strcmp(str, "CONFIG_DISP_DATA_EEPROM_ONLY") == 0)
		return CONFIG_DISP_DATA_EEPROM_ONLY;
	else if(strcmp(str, "CONFIG_DISP_DATA_SD_ONLY") == 0)
		return CONFIG_DISP_DATA_SD_ONLY;
	else if(strcmp(str, "CONFIG_DISP_DATA_EEPROM_SD") == 0)
		return CONFIG_DISP_DATA_EEPROM_SD;
	else
		return  CONFIG_DISP_DATA_SD_EEPROM;
};

enum i2c_mode_id get_i2c_mode(char* str){
	if(strcmp(str, "I2C_MODE_HOST") == 0)
		return I2C_MODE_HOST;
	else if(strcmp(str, "I2C_MODE_DISP") == 0)
		return I2C_MODE_DISP;
	else if(strcmp(str, "I2C_MODE_S1D13524") == 0)
		return I2C_MODE_S1D13524;
	else if(strcmp(str, "I2C_MODE_SC18IS6XX") == 0)
		return I2C_MODE_SC18IS6XX;
	else
		return I2C_MODE_NONE;
};

int read_config(char* configfile, struct config *config){
	FIL cfg;
	int stat = 0;
	if(config == NULL)
		config = (struct config*) malloc(sizeof(struct config));

	if (f_open(&cfg, configfile, FA_READ) != FR_OK) {
		LOG("Failed to open config text file [%s]", configfile);
		return -1;
	}
	char line[81];
	int len;
	long int lno;
	char config_name[16];
	while(!stat){
		++lno;

		stat = parser_read_file_line(&cfg, line, sizeof(line));

		if (stat < 0) {
			LOG("Failed to read line");
			break;
		}

		if (!stat) {
			f_lseek(&cfg, 0);
			lno = 0;
			continue;
		}

		stat = 0;
		if ((line[0] == '\0') || (line[0] == '#'))
			continue;
		LOG("%s, %i", line, stat);

		len = parser_read_str(line, SEP, config_name, sizeof(config_name));

		if (len < 0) {
			LOG("Failed to config");
			stat = -1;
			break;
		}

		if(config->i2c_mode == NULL && strcmp(config_name, "i2c_mode")==0){
			char i2c_mode[16] = {0,};
			len = parser_read_str(&line[len], SEP, i2c_mode, sizeof(i2c_mode));
			// evaluate string
			config->i2c_mode = get_i2c_mode(i2c_mode);

		}else if(strcmp(config_name, "data_source")==0){
			char data_source[16] = {0,};
			len = parser_read_str(&line[len], SEP, data_source, sizeof(data_source));
			// evaluate string
			config->data_source = get_data_source(data_source);

		}else if(strcmp(config_name, "board")==0){
			char board[16] = {0,};
			len = parser_read_str(&line[len], SEP, board, sizeof(board));
			// evaluate string
			config->board = get_board(board);

		}else if(strcmp(config_name, "display_type")==0){
			char display_type[16] = {0,};
			len = parser_read_str(&line[len], SEP, config->config_display_type, sizeof(display_type));
			// evaluate string
			LOG("%s", display_type);
		}else if(strcmp(config_name, "endianess")==0){
			char endianess[16];
			len = parser_read_str(&line[len], SEP, endianess, sizeof(endianess));
			// evaluate string
			config->endianess = get_endianess(endianess);
		}else if(strcmp(config_name, "wf_version")==0){
			len = parser_read_int(&line[len], SEP, &config->waveform_version);
		}else{
			f_close(&cfg);
			return 0;
		}
	}
	f_close(&cfg);
	return stat;
}
