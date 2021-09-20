/*
 * L9906.h
 *
 *  Created on: Sep 3, 2021
 *      Author: RD02995
 */

#ifndef L9906_H_
#define L9906_H_

#include "driver/spi_master.h"
#include "driver/gpio.h"


#define DRV_CS GPIO_NUM_18 //zong
#define DRV_SCLK GPIO_NUM_5//hong
#define DRV_MOSI GPIO_NUM_17//U2TXD//cheng
#define DRV_MISO GPIO_NUM_16//U2RXD//huang

#define CMD0 0x0//b000
#define CMD1 0x01//b001
#define CMD2 0x02//b010
#define CMD3 0x03//b011
#define CMD4 0x04//b100

#define CMD0_Off1 0//current sense amplifier offset: ground: 0; phase: 1
#define CMD0_G1 0 //current sense amplifier Gain: 10(0),30(1),50(2),100(3)
#define CMD0_Off2 0 //current sense amplifier offset: ground: 0; phase: 1
#define CMD0_G2 0
#define CMD0_IG 0 // IG = 25%(0),50%(1),75%(2),100%(3)
#define CMD0_DT 3 // 100-200ns(0),300-500ns(1),700-1000ns(2),1000-1500ns(3)
#define CMD0_WE 1
#define CMD0_PAR_WR 0

#define CMD1_VCCov 1//Vcc over voltage threshold 3.3V(2) 5v(1) -B0-B1
#define CMD1_VBov 1 //Battery Voltage over voltage threshold 27.5V-34.5V(2),36-42V(1) -B2-B3
#define CMD1_SCHS 0 //Short circuit detection threshold for high-side external MOSFET  -B4-B5
#define CMD1_SCLS 0 //Short circuit detection threshold for low-side external MOSFET  -B6-B7
#define CMD1_EN_VBuv 1//B8
#define CMD1_EN_VBov 1//B(9)
#define CMD1_WE 1//B10
#define CMD1_PAR_WR 0//B11
#define CMD1_EN_THSD 1//B12
/*
0.4-0.6V(0)
0.7-0.9V(1)
0.9V-1.1V(2)
1.17V-1.43V(3)
*/
#define ShortPH 0  //external FET short detected
#define VOVTST 0 // Test function for over voltage level
#define VSCTST 0 // Test function for short circuit level
#define SHT_PH 0// shoot-through detect
#define BST_DIS_EN 0//BST_Disable_function
#define GCR_INT_I 0
#define WE 1

#define DIS_BSTov 0
#define TRIM2 0x1F
#define TRIM1 0x10

#define EN_FAULT 0x3FF
#define REGOFF_EN 0


#define WR_CMD0 CMD0_Off1+(CMD0_G1<<1)+(CMD0_Off2<<3)+(CMD0_G2<<4)+(CMD0_IG<<6)+(CMD0_DT<<8)+(CMD0_WE<<10)+(CMD0_PAR_WR<<11)
#define WR_CMD1 CMD1_VCCov+(CMD1_VBov<<2)+(CMD1_SCHS<<4)+(CMD1_SCLS<<6)+(CMD1_EN_VBuv<<8)+(CMD1_EN_VBov<<9)+(CMD1_WE<<10)+(CMD1_PAR_WR<<11)+(CMD1_EN_THSD<<12)
#define WR_CMD2 (ShortPH<<5) + (VOVTST<<6) + (VSCTST<<7) + (SHT_PH<<8) + (BST_DIS_EN<<9) + (GCR_INT_I<<12) + (WE<<10) + (1<<11)
#define WR_CMD3 TRIM1 + (TRIM2<<5) + (WE<<10) + (0<<11) + (DIS_BSTov<<12)
#define WR_CMD4 EN_FAULT+(WE<<10)+(1<<11)+(REGOFF_EN<<12)
#define RD_CMD0 0+(0<<1)+(0<<3)+(0<<4)+(0<<6)+(0<<8)+(0<<10)+(1<<11)

typedef struct {
    spi_device_handle_t spi;
}predrv_context_t;

typedef struct predrv_context_t* predrv_handle_t;

void predrv_spi_init(predrv_context_t** out_ctx);
int wr_predrv(predrv_context_t* ctx, int cmd, int data);
int rd_predrv(predrv_context_t* ctx, int cmd);

#endif /* L9906_H_ */
