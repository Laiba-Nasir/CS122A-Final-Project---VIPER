// SPI transaction to FPGA
#include "pico/stdlib.h"
#include "hardware/spi.h"

// we will be using SPI code to coordinatev the byte order
//pico should send a 1-byte read request to the FPGA(icesugar pro), and the FPGA should respond with 5 bytes
#define CS 17
#define SCK 18
#define MOSI 19


int main(){
    stdio_init_all();

    //initialize our SPI
    spi_init(spi0, 1000 * 1000);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(MOSI, GPIO_FUNC_SPI);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1); // set CS high

    return 0;
}