// SPI, servos, laser, buzzer
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "ov7670_regs.h"
#include "spi_master.h"

//extern init_ready from our I2C file so that we can check if our camera is done with initialization
extern bool init_ready;
//extern our tick function and ov7670_setup so that we can use them
extern bool Ov7670_TICK(struct repeating_timer *t);
extern void ov7670_setup();

fpga_centroid centroid = {0, 0, false}; //we need to initialize our centroid struct

//main files repeating timer function will go here
bool main_TICK(struct repeating_timer *t){
    //read data from the camera + fpga
    //(fill out later)
    if(centroid_read(&centroid)){
        //we need to check if a color was detected
        if(centroid.color_detected){
            //print the coordinates of the color that was detected
            printf("[COLOR DETECTED] x: %d, y: %d\n", centroid.x, centroid.y);
        }else{
            //if there is no color or if it lost tracking, print out a message
            printf("[NO COLOR DETECTED]\n");
        }
    }else{
        //if we don't read data successfully, print out an error message
        printf("[ERROR] Failedto read data from camera\n");
    }

    return true;
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
    spi_master_init();

    //initialize main timer
    struct repeating_timer main_timer;
    add_repeating_timer_ms(-33, main_TICK, NULL, &main_timer);

    //keep main alive so that our camera and main timer keep running
    while(true){}

}