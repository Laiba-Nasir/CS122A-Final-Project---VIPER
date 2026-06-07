// read_centroid() header
#ifndef SPI_MASTER_H
#define SPI_MASTER_H

#include <stdint.h>
#include <stdbool.h>

//seperate header file from our .c code to make it easier to call our spi logic in main.c
typedef struct{
    uint16_t centroid_x;
    uint16_t centroid_y;
    bool color_detected;
} fpga_centroid;

void spi_master_init(void);
bool centroid_read(fpga_centroid* centroid);

#endif // SPI_MASTER_H