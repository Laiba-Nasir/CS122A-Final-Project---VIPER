// SPI, servos, laser, buzzer
#include "pico/stdlib.h"
#include "ov7670_init.c"
#include "spi_master.c"

//main files repeating timer function will go here
bool main_TICK(struct repeating_timer *t){
    //read data from the camera + fpga
    //(fill out later)
}

int main() {
    stdio_init_all();

    //call our ov7670 setup function 
    ov7670_setup();

    //start our repeating timer for our camera
    struct repeating_timer camera_timer;
    add_repeating_timer_ms(-1, Ov7670_TICK, NULL, &camera_timer); //we will be using a negative delay to make sure that our timer starts immediately

    //we need to wait until our camera is done initializing before we can start our main timer
    while(!init_ready){ }

    //initialize our spi

    //initialize main timer
    struct repeating_timer main_timer;
    add_repeating_timer_ms(-33, main_TICK, NULL, &main_timer);

}