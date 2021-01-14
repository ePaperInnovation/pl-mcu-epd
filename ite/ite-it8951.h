/*
 * ite-it8951.h
 *
 *  Created on: 04.01.2021
 *      Author: oliver.lenz
 */

#ifndef ITE_ITE_IT8951_H_
#define ITE_ITE_IT8951_H_

#include <pl/epdc.h>
#include <pl/interface.h>
#include <stdint.h>
#include <stdlib.h>
#include <intrinsics.h>
#include <ite/ite-epdc.h>


//Built in I80 Command Code
#define IT8951_TCON_SYS_RUN      0x0001
#define IT8951_TCON_STANDBY      0x0002
#define IT8951_TCON_SLEEP        0x0003
#define IT8951_TCON_REG_RD       0x0010
#define IT8951_TCON_REG_WR       0x0011
#define IT8951_TCON_MEM_BST_RD_T 0x0012
#define IT8951_TCON_MEM_BST_RD_S 0x0013
#define IT8951_TCON_MEM_BST_WR   0x0014
#define IT8951_TCON_MEM_BST_END  0x0015
#define IT8951_TCON_LD_IMG       0x0020
#define IT8951_TCON_LD_IMG_AREA  0x0021
#define IT8951_TCON_LD_IMG_END   0x0022
#define IT8951_TCON_BYPASS_I2C   0x003E

//I80 User defined command code
#define USDEF_I80_CMD_DPY_AREA     0x0034
#define USDEF_I80_CMD_GET_DEV_INFO 0x0302
#define USDEF_I80_CMD_POWER_CTR         0x0038
#define USDEF_I80_CMD_VCOM_CTR          0x0039
#define USDEF_I80_CMD_FORCE_SET_TEMP    0x0040


//Panel
#define IT8951_PANEL_WIDTH   1024 //it Get Device information
#define IT8951_PANEL_HEIGHT   758

//Rotate mode
#define IT8951_ROTATE_0     0
#define IT8951_ROTATE_90    1
#define IT8951_ROTATE_180   2
#define IT8951_ROTATE_270   3

//Pixel mode , BPP - Bit per Pixel
#define IT8951_2BPP   0
#define IT8951_3BPP   1
#define IT8951_4BPP   2
#define IT8951_8BPP   3

//Endian Type
#define IT8951_LDIMG_L_ENDIAN   0
#define IT8951_LDIMG_B_ENDIAN   1

//-----------------------------------------------------------------------
// IT8951 TCon Registers defines
//-----------------------------------------------------------------------
//Register Base Address
#define DISPLAY_REG_BASE 0x1000               //Register RW access for I80 only
//Base Address of Basic LUT Registers
#define LUT0EWHR  (DISPLAY_REG_BASE + 0x00)   //LUT0 Engine Width Height Reg
#define LUT0XYR   (DISPLAY_REG_BASE + 0x40)   //LUT0 XY Reg
#define LUT0BADDR (DISPLAY_REG_BASE + 0x80)   //LUT0 Base Address Reg
#define LUT0MFN   (DISPLAY_REG_BASE + 0xC0)   //LUT0 Mode and Frame number Reg
#define LUT01AF   (DISPLAY_REG_BASE + 0x114)  //LUT0 and LUT1 Active Flag Reg
//Update Parameter Setting Register
#define UP0SR (DISPLAY_REG_BASE + 0x134)      //Update Parameter0 Setting Reg

#define UP1SR     (DISPLAY_REG_BASE + 0x138)  //Update Parameter1 Setting Reg
#define LUT0ABFRV (DISPLAY_REG_BASE + 0x13C)  //LUT0 Alpha blend and Fill rectangle Value
#define UPBBADDR  (DISPLAY_REG_BASE + 0x17C)  //Update Buffer Base Address
#define LUT0IMXY  (DISPLAY_REG_BASE + 0x180)  //LUT0 Image buffer X/Y offset Reg
#define LUTAFSR   (DISPLAY_REG_BASE + 0x224)  //LUT Status Reg (status of All LUT Engines)

#define BGVR      (DISPLAY_REG_BASE + 0x250)  //Bitmap (1bpp) image color table
//-------System Registers----------------
#define SYS_REG_BASE 0x0000

//Address of System Registers
#define I80CPCR (SYS_REG_BASE + 0x04)
//-------Memory Converter Registers----------------
#define MCSR_BASE_ADDR 0x0200
#define MCSR (MCSR_BASE_ADDR  + 0x0000)
#define LISAR (MCSR_BASE_ADDR + 0x0008)

struct pl_gpio;
struct pl_wflib;

struct it8951_data {
    unsigned cs0;
    unsigned hrdy;
};

struct it8951 {
    const struct it8951_data *data;
    struct pl_gpio *gpio;
    struct pl_interface *interface;
    uint16_t scrambling;
    uint16_t source_offset;
    uint16_t hrdy_mask;
    uint16_t hrdy_result;
    int measured_temp;
    unsigned xres;
    unsigned yres;
    uint32_t imgBufBaseAdrr;
    struct {
        uint8_t needs_update:1;
    } flags;
};


typedef struct IT8951LdImgInfo
{
    uint16_t usEndianType; //little or Big Endian
    uint16_t usPixelFormat; //bpp
    uint16_t usRotate; //Rotate mode
    uint32_t ulStartFBAddr; //Start address of source Frame buffer
    uint32_t ulImgBufBaseAddr;//Base address of target image buffer

}IT8951LdImgInfo;

typedef struct IT8951AreaImgInfo
{
    uint16_t usX;
    uint16_t usY;
    uint16_t usWidth;
    uint16_t usHeight;

}IT8951AreaImgInfo;

typedef struct
{
    uint16_t usPanelW;
    uint16_t usPanelH;
    uint16_t usImgBufAddrL;
    uint16_t usImgBufAddrH;
    uint16_t usFWVersion[8]; //16 Bytes String
    uint16_t usLUTVersion[8]; //16 Bytes String

}I80IT8951DevInfo;

void it8951_load_init_code(struct it8951 *p, I80IT8951DevInfo *pBuf);
extern int it8951_clear_init(struct it8951 *p);
extern int it8951_update(struct it8951 *p, int wfid, enum pl_update_mode mode, const struct pl_area *area);
extern int it8951_set_power_state(struct it8951 *p, enum pl_epdc_power_state state);
extern int it8951_set_epd_power(struct it8951 *p, int on);
extern int it8951_load_image(struct it8951 *p, const char *path, uint16_t mode, unsigned bpp, struct pl_area *area, int left, int top);
extern int it8951_wait_idle(struct it8951 *p);
extern int it8951_wait_update_end(struct it8951 *p);
extern void it8951_cmd(struct it8951 *p, uint16_t cmd, const uint16_t *params, size_t n);
extern uint16_t it8951_read_reg(struct it8951 *p, uint16_t reg);
extern void it8951_write_reg(struct it8951 *p, uint16_t reg, uint16_t val, int size);
extern void it8951_update_Temp(struct it8951 *p, int tempMode, int temp);
extern int waitForHRDY(struct it8951 *p);
extern int it8951_waitForDisplayReady(struct it8951 *p);
extern void it8951_setVcom(struct it8951 *p, int vcom);
extern int it8951_fill(struct it8951 *p, const struct pl_area *area, uint8_t g);

//extern void it8951_hostAreaPackedPixelWrite(struct it8951 *p, );



#endif /* ITE_ITE_IT8951_H_ */
