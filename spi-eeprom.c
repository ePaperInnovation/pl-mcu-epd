/*
 * spi-eeprom.c
 *
 *  Created on: 25.01.2021
 *      Author: oliver.lenz
 */

#include <spi-eeprom.h>
#include <msp430/msp430-spi.h>
#include <pl/dispinfo.h>
#define LOG_TAG "spi-eeprom"

struct eeprom_data
{
    uint16_t size; /* size in bytes (-1) */
    uint8_t page_size; /* size of a page in bytes */
    uint8_t flags; /* flags from i2c_eeprom_flags */
};

struct pl_dispinfo_info *dispInfo;

static int send_cmd(uint8_t cmd);
static uint32_t get4Bytes(uint8_t *buffer, uint32_t addr);

int send_cmd(uint8_t cmd)
{
    int stat = 0;
    //reInitSPI(p, 4);
    //set CS low TODO:Make nicer !
    //P3OUT &= ~BIT6;
    //int stat = p->interface->write(&cmd, 1);
    stat = msp430_spi_write_bytes(&cmd, 1);
    //set CS high
    //P3OUT |= BIT6;
    //reInitSPI(p, 2);
    return stat;
}

int nvm_MX25_spi_read(unsigned int addr, struct pl_dispinfo *blob, uint32_t len)
{

    //assert(blob);

    int stat = 0;
    uint32_t register_address = addr;
    uint8_t reg[3];
    int chunkSize = 256;
    uint32_t byte_offset = 0;
    uint32_t bytes_to_transfer = len;
    uint8_t *data;
//
//    //set SPI Speed to 1MHz
//    // reInitSPI(p, 20);
//
    // read chip id
    uint8_t rdid_data[3];

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_RDID);
    stat = msp430_spi_read_bytes(rdid_data, 1);
    stat = msp430_spi_read_bytes(rdid_data + 1, 1);
    stat = msp430_spi_read_bytes(rdid_data + 2, 1);
    P3OUT |= BIT6;

    printf("Manufacturing ID NVM: %x, %x, %x", rdid_data[0], rdid_data[1],
           rdid_data[2]);

//    while (bytes_to_transfer > 0)
//    {
//
//        // transfer chunkSize or bytes to transfer
//        size_t transferChunkSize =
//                (bytes_to_transfer >= chunkSize) ?
//                        chunkSize : bytes_to_transfer;
//
//        reg[0] = (register_address >> 16) & 0xff;
//        reg[1] = (register_address >> 8) & 0xff;
//        reg[2] = (uint8_t) register_address;
//
//        P3OUT &= ~BIT6;
//        stat = send_cmd(MX25U4033E_READ);         // read command
//        stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
//        stat = msp430_spi_read_bytes(data, transferChunkSize);      // read data
//        P3OUT |= BIT6;
//
//        byte_offset += transferChunkSize;
//        register_address += transferChunkSize;
//        bytes_to_transfer -= transferChunkSize;
//    }

    uint8_t dispID[NVM_DISPID_LEN];

    reg[0] = (NVM_DISPID_POS >> 16) & 0xff;
    reg[1] = (NVM_DISPID_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_DISPID_POS;
    int count = 0;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    for (count = 0; count < NVM_DISPID_LEN; count++)
    {
        stat = msp430_spi_read_bytes(dispID + count, 1);      // read data
    }
    P3OUT |= BIT6;

    uint8_t prodID[NVM_PRODID_LEN];

    reg[0] = (NVM_PRODID_POS >> 16) & 0xff;
    reg[1] = (NVM_PRODID_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_PRODID_POS;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    stat = msp430_spi_read_bytes(prodID, NVM_PRODID_LEN);      // read data
    P3OUT |= BIT6;

    uint8_t vcomRead[NVM_VCOM_LEN];

    reg[0] = (NVM_VCOM_POS >> 16) & 0xff;
    reg[1] = (NVM_VCOM_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_VCOM_POS;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    stat = msp430_spi_read_bytes(vcomRead, NVM_VCOM_LEN);      // read data
    P3OUT |= BIT6;

    int32_t extractedVCom = get4Bytes(vcomRead, 0x00);

    dispInfo->vcom = extractedVCom;

    free(data);

    return stat;
}

static uint32_t get4Bytes(uint8_t *buffer, uint32_t addr)
{

    uint32_t value = 0;

    value = (int) (buffer[addr] << 24);
    value |= (int) (buffer[addr + 1] << 16);
    value |= (int) (buffer[addr + 2] << 8);
    value |= (int) (buffer[addr + 3]);

    return value;
}
