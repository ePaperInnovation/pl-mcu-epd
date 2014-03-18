#ifndef INCLUDE_CONFIG_H
#define INCLUDE_CONFIG_H 1
#define CONFIG_LITTLE_ENDIAN          1
#define CONFIG_HWINFO_EEPROM          1
#define CONFIG_HWINFO_DEFAULT         1
#define CONFIG_PLAT_RAVEN             1 /**< Raven board */
#define CONFIG_PLAT_Z13               0 /**< Hummingbird Z1.3 board */
#define CONFIG_PLAT_Z6                0 /**< Hummingbird Z6.x board */
#define CONFIG_PLAT_Z7                0 /**< Hummingbird Z7.x board */
#define CONFIG_DEFAULT_I2C_MODE       I2C_MODE_HOST
#define CONFIG_DISP_DATA_EEPROM_ONLY  1 /**< Only use display EEPROM */
#define CONFIG_DISP_DATA_SD_ONLY      0 /**< Only use SD card */
#define CONFIG_DISP_DATA_EEPROM_SD    0 /**< Try EEPROM first, then SD card */
#define CONFIG_DISP_DATA_SD_EEPROM    0 /**< Try SD card first, then EEPROM */
#define CONFIG_DISPLAY_TYPE           "Type11"
#define CONFIG_DEMO_POWERMODES        0
#endif /* INCLUDE_CONFIG_H */
