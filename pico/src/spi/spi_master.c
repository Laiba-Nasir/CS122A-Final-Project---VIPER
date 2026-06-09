// SPI transaction to FPGA
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "spi_master.h"

// we will be using SPI code to coordinatev the byte order
//pico should send a 1-byte read request to the FPGA(icesugar pro), and the FPGA should respond with 5 bytes
#define MISO 16
#define CS 17
#define SCK 18
#define MOSI 19

//seperate our spi initialization and read logic
void spi_master_init(void){
    //initialize our SPI
    spi_init(spi0, 1000 * 1000);

    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1); // set CS high
}

bool centroid_read(fpga_centroid* centroid){
    //we will be using dummy data, especially for tx and rx
    uint8_t read_req = 0x01; //as stated in our packet, the pico needs to send one byte read request
    //we need have 1 real byte and 4 dummy bytes for the FPGA to be able to respond with 5 bytes
    uint8_t rx[6] = {0x01, 0, 0, 0, 0, 0}; //the FPGA needs to respond with 5 bytes
    uint8_t tx[5] = {0}; //we need to send 5 bytes but we only care about the first byte

    //we need to make sure our CS is low
    gpio_put(CS, 0);
    
    //send the read request to the FPGA
    spi_write_blocking(spi0, &read_req, 1);

    //read the 5 byte response from the fpga
    spi_write_read_blocking(spi0, tx, rx, 5);

    //set our CS back to high
    gpio_put(CS, 1);

    //now we need to reconstruct our data from the rx buffer
    centroid->centroid_x = (rx[1] << 8) | rx[2]; //basically, we are combining the high and low byte
    centroid->centroid_y = (rx[3] << 8) | rx[4]; //same thing for y
    centroid->color_detected = rx[5] & 0x01; //our last byte will check whether a color has been detected

    return true;
}