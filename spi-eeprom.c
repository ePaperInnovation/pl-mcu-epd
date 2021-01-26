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

static int send_cmd(uint8_t cmd);
uint32_t get4Bytes(uint8_t *buffer, uint32_t addr);

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

int nvm_MX25_spi_read(unsigned int addr, struct pl_dispinfo *dispInfo,
                      uint32_t len)
{

    //assert(blob);

    int stat,z = 0;
    uint32_t register_address = addr;
    uint8_t reg[3];
    int chunkSize = 256;
    uint32_t byte_offset = 0;
    uint32_t bytes_to_transfer = len;
    uint8_t *data;

    // read chip id
    uint8_t rdid_data[3];

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_RDID);
    stat = msp430_spi_read_bytes(rdid_data, 1);
    stat = msp430_spi_read_bytes(rdid_data + 1, 1);
    stat = msp430_spi_read_bytes(rdid_data + 2, 1);
    P3OUT |= BIT6;

    printf("%-16s " "Manufacturing ID NVM: %x, %x, %x" "\n", "spi-eeprom",
           rdid_data[0], rdid_data[1], rdid_data[2]);

    uint8_t magicID[NVM_MAGIC_ID_LEN];

    reg[0] = (NVM_MAGIC_ID_POS >> 16) & 0xff;
    reg[1] = (NVM_MAGIC_ID_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_MAGIC_ID_POS;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    stat = msp430_spi_read_bytes(magicID, NVM_MAGIC_ID_LEN);      // read data
    P3OUT |= BIT6;

    uint32_t test = 0x00000000;
    test |= magicID[0] << 8;
    test |= magicID[1];

    dispInfo->vermagic.magic = (uint32_t) test;

    printf("%-16s " "Magic-Word: %x" "\n", "spi-eeprom",
           dispInfo->vermagic.magic);

    uint8_t nvmVer[NVM_NVMVERS_LEN];

    reg[0] = (NVM_NVMVERS_POS >> 16) & 0xff;
    reg[1] = (NVM_NVMVERS_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_NVMVERS_POS;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    stat = msp430_spi_read_bytes(nvmVer, NVM_NVMVERS_LEN);      // read data
    P3OUT |= BIT6;

    uint8_t a = nvmVer[5];
    uint8_t high_bits = a>>4;
    uint8_t low_bits = a&15;

    dispInfo->vermagic.version = low_bits;

    printf("%-16s " "NVM-Version: %i" "\n", "spi-eeprom",
           dispInfo->vermagic.version);

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


    for (z = 0; z < sizeof(dispInfo->info.panel_id); z++)
    {
        dispInfo->info.panel_id[z] = dispID[z];
    }

    printf("%-16s " "Panel-ID: %s" "\n", "spi-eeprom", dispInfo->info.panel_id);

    uint8_t prodID[NVM_PRODID_LEN];

    reg[0] = (NVM_PRODID_POS >> 16) & 0xff;
    reg[1] = (NVM_PRODID_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_PRODID_POS;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    stat = msp430_spi_read_bytes(prodID, NVM_PRODID_LEN);      // read data
    P3OUT |= BIT6;

    for (z = 0; z < sizeof(dispInfo->info.panel_type); z++)
    {
        dispInfo->info.panel_type[z] = prodID[z];
    }

    printf("%-16s " "Panel-Type: %s" "\n", "spi-eeprom",
           dispInfo->info.panel_type);

    uint8_t vcomRead[NVM_VCOM_LEN];

    reg[0] = (NVM_VCOM_POS >> 16) & 0xff;
    reg[1] = (NVM_VCOM_POS >> 8) & 0xff;
    reg[2] = (uint8_t) NVM_VCOM_POS;

    P3OUT &= ~BIT6;
    stat = send_cmd(MX25U4033E_READ);         // read command
    stat = msp430_spi_write_bytes(reg, 3);           // write 3-byte address
    stat = msp430_spi_read_bytes(vcomRead, NVM_VCOM_LEN);      // read data
    P3OUT |= BIT6;

    dispInfo->info.vcom = get4Bytes(vcomRead, 0x00);

    printf("%-16s " "Read VCom: %i" "\n", "spi-eeprom", dispInfo->info.vcom);

    memset(dispInfo->info.waveform_md5, 0xFF, sizeof dispInfo->info.waveform_md5);
    dispInfo->info.waveform_full_length = 0;
    dispInfo->info.waveform_lzss_length = 0;
    dispInfo->info.waveform_id[0] = '\0';
    dispInfo->info.waveform_target[0] = '\0';

    free(data);

    return stat;
}

uint32_t get4Bytes(uint8_t *buffer, uint32_t addr)
{

    uint32_t value = 0;

    value = (int) (buffer[addr] << 24);
    value |= (int) (buffer[addr + 1] << 16);
    value |= (int) (buffer[addr + 2] << 8);
    value |= (int) (buffer[addr + 3]);

    return value;
}
