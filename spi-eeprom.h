/*
 * spi-eeprom.h
 *
 *  Created on: 25.01.2021
 *      Author: oliver.lenz
 */

#ifndef SPI_EEPROM_H_
#define SPI_EEPROM_H_

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <intrinsics.h>
#include <msp430/msp430-spi.h>
#include <utils.h>
#include <ite/ite-it8951.h>


// Size
#define MX25U4033E_SIZE     (0x80000)   // max size

// Register Address
#define MX25U4033E_PP       (0x02)  // page program
#define MX25U4033E_READ     (0x03)  // read
#define MX25U4033E_RDSR     (0x05)  // read status register
#define MX25U4033E_WREN     (0x06)  // write enable
#define MX25U4033E_SE       (0x20)  // sector erase
#define MX25U4033E_CE       (0x60)  // chip erase
#define MX25U4033E_RDID     (0x9f)  // read identification

#define MX25U4033E_MANID_DEVID (0xc22018) // (0xc22013) 1xbyte Manufacture id 2xbyte device id

// Status Register Bits
#define MX25U4033E_STATUS_WIP   (1     ) // write in progress
#define MX25U4033E_STATUS_WEL   (1 << 1) // write enable latch

#define NVM_PRODID_POS      0x70000
#define NVM_PRODID_LEN      0x00010
#define NVM_VCOM_POS        0x70010
#define NVM_VCOM_LEN        0x00010
#define NVM_WFVERS_POS      0x70020
#define NVM_WFVERS_LEN      0x00020
#define NVM_FPLVERS_POS     0x70040
#define NVM_FPLVERS_LEN     0x00010
#define NVM_DISPID_POS      0x70050
#define NVM_DISPID_LEN      0x00030
#define NVM_NVMVERS_POS     0x70080
#define NVM_NVMVERS_LEN     0x00010
#define NVM_FEATURE1_POS    0x70090
#define NVM_FEATURE2_POS    0x70094
#define NVM_FEATURE3_POS    0x70098
#define NVM_FEATURE4_POS    0x7009C
#define NVM_FEATURE_LEN     0x00004
#define NVM_WF_START_POS    0x700A0 // 4 bytes
#define NVM_WF_LEN_POS      0x700A4 // 4 bytes
#define NVM_MAGIC_ID_POS    0x70080
#define NVM_MAGIC_ID_LEN    0x00002
#define NVM_MAGIC_ID        0x504C //PL
#define NVM 0xffff;

extern int nvm_MX25_spi_read(unsigned int addr, struct pl_dispinfo *blob,
                             uint32_t len);

#endif /* SPI_EEPROM_H_ */
