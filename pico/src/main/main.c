// SPI, servos, laser, buzzer
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "ov7670_regs.h"
#include "spi_master.h"
#include "servo.h"

#define LASER_PIN 22 //THIS IS NOT THE OFFICIAL LASER PIN   

#define FRAME_WIDTH     640
#define FRAME_HEIGHT    480

//you need to determine the k value erick
#define K

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
            //turn laser on if a color is detected
            gpio_put(LASER_PIN, 1);

            //now, we need to calculate the peripherals error offset (from the servo)
            //The code goes here Erick :D!
            //float error_x = (float)centroid.centroid_x - (FRAME_WIDTH / 2.0f);
            //float error_y = (float)centroid.centroid_y - (FRAME_HEIGHT/2.0f);

            //FOR ERICK: put the code for adjusting the angles of the pan and tilt here
            
            //FOR ERICK: You can put your clamp code here as well :D

            //FOR ERICK: you can put your servo_write code with the adjusted pan and tilt angles here if ya want


            //print the coordinates of the color that was detected
            printf("[COLOR DETECTED] x: %d, y: %d\n", centroid.centroid_x, centroid.centroid_y);
        }else{
            gpio_put(LASER_PIN, 0);
            //if there is no color or if it lost tracking, print out a message
            printf("[NO COLOR DETECTED]\n");
        }
    }else{
        //turn off laser
        gpio_put(LASER_PIN, 0);
        //if we don't read data successfully, print out an error message
        printf("[ERROR] Failed to read data from camera\n");
    }

    return true;
}

int main() {
    stdio_init_all();

    //call our ov7670 setup function 
    //make sure to also call the servo init() file
    ov7670_setup();

    //start our repeating timer for our camera
    struct repeating_timer camera_timer;
    add_repeating_timer_ms(-1, Ov7670_TICK, NULL, &camera_timer); //we will be using a negative delay to make sure that our timer starts immediately

    //we need to wait until our camera is done initializing before we can start our main timer
    while(!init_ready){ }

    //initialize our spi
    spi_master_init();

    //servos
    //these were retrieved from Erick's branch
    servo_init(PAN_PIN);
    servo_init(TILT_PIN);
 
    // Center both axes, then pause to let them settle.
    servo_write(PAN_PIN, 90);
    servo_write(TILT_PIN, 90);
    sleep_ms(500);

    //laser
    gpio_init(LASER_PIN);
    gpio_set_dir(LASER_PIN, GPIO_OUT);
    gpio_put(LASER_PIN, 0);

    //initialize main timer
    struct repeating_timer main_timer;
    add_repeating_timer_ms(-33, main_TICK, NULL, &main_timer);

    //keep main alive so that our camera and main timer keep running
    while(true){}

}