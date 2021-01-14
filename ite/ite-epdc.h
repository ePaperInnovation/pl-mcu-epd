/*
 * ite-epdc.h
 *
 *  Created on: 04.01.2021
 *      Author: oliver.lenz
 */

#ifndef ITE_ITE_EPDC_H_
#define ITE_ITE_EPDC_H_

struct pl_epdc;
struct pl_dispinfo;
struct it8951;

#define  MY_WORD_SWAP(x) ( ((x & 0xff00)>>8) | ((x & 0x00ff)<<8) )

extern int ite_epdc_init(struct pl_epdc *epdc,
               const struct pl_dispinfo *dispinfo,
               struct it8951 *it8951);

//extern int epson_epdc_init_s1d13541(struct pl_epdc *epdc);

#endif /* ITE_ITE_EPDC_H_ */
