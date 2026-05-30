// SPI transaction to FPGA
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// we will be using SPI code to coordinatev the byte order
//pico should send a 1-byte read request to the FPGA(icesugar pro), and the FPGA should respond with 5 bytes
#define MISO 16
#define CS 17
#define SCK 18
#define MOSI 19

// we need a data struct 
typedef struct{
    uint16_t centroid_x;
    uint16_t centroid_y;
    bool color_detected;
} fpga_centroid;

bool centroid_read(fpga_centroid* centroid){
    //we will be using dummy data, especially for tx and rx
    uint8_t read_req = 0x01; //as stated in our packet, the pico needs to send one byte read request
    uint8_t rx[5] = {0}; //the FPGA needs to respond with 5 bytes
    uint8_t dummy_tx[5] = {0}; //our dummy data for the tx

    //we need to make sure our CS is low
    gpio_put(CS, 0);
    
    //send the read request to the FPGA
    spi_write_blocking(spi0, &read_req, 1);

    //read the 5 byte response from the fpga
    spi_write_read_blocking(spi0, dummy_tx, rx, 5);

    //set our CS back to high
    gpio_put(CS, 1);

    //now we need to reconstruct our data from the rx buffer
    centroid->centroid_x = (rx[0] << 8) | rx[1]; //basically, we are combining the high and low byte
    centroid->centroid_y = (rx[2] << 8) | rx[3]; //same thing for y
    centroid->color_detected = rx[4] & 0x01; //our last byte will check whether a color has been detected

    return true;
}

int main(){
    stdio_init_all();

    //initialize our SPI
    spi_init(spi0, 1000 * 1000);

    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MISO, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1); // set CS high

    fpga_centroid curr_centroid = {0, 0, false};

    //we need to coninously read the centroid data from the FPGA and print it out
    while(true){
        if(centroid_read(&curr_centroid)){
            //if our traget was detected, we need to print its coordinates
            if(curr_centroid.color_detected){
                printf("[TARGET DETECTED] x-coordinates: %d, y_coordinates: %d \n", curr_centroid.centroid_x, curr_centroid.centroid_y);
            }else{
                printf("[TARGET LOST] Searching... \n");
            }
        }else{
            printf("[ERROR] couldn't read from FPGA \n");
        }
    }

    return 0;
}