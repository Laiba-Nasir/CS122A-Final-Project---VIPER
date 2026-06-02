// register address/value table
//we will be using the values from the datasheet: https://www.voti.nl/docs/OV7670.pdf

#ifndef OV7670_REGS_H
#define OV7670_REGS_H

#include <stdint.h>

//all of our register values are from the data sheet pages 10-23.
//they will be in hex
#define OV7670_REG_COM7         0x12
#define OV7670_REG_COM15        0x40
#define OV7670_REG_COM10        0x15
#define OV7670_REG_COM14        0x3E
#define OV7670_REG_COM9         0x14
#define OV7670_REG_COM8         0x13
#define OV7670_REG_COM3         0x0C
#define OV7670_REG_COM2         0x09


#define OV7670_REG_RGB444       0x8C  //this is a safety net. We will be using RGB565, but we need to make sure the camera knows not to used RGB444

#define OV7670_REG_TSLB         0x3A
#define OV7670_REG_CLKRC        0x11
#define OV7670_REG_MVFP         0x1E

#define OV7670_REG_HSTART       0x17
#define OV7670_REG_HSTOP        0x18
#define OV7670_REG_HREF         0x32

#define OV7670_REG_VSTART       0x19
#define OV7670_REG_VSTOP        0x1A
#define OV7670_REG_VREF         0x03

#define OV7670_REG_TABLE_END    0xFF

//the register values that we will be writing in ov7670_init.c
extern const uint8_t ov7670_init_regs[][2];


#endif