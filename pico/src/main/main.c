// SPI, servos, laser, buzzer
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include "ov7670_regs.h"
#include "spi_master.h"
#include "servo.h" 

#define FRAME_WIDTH     640
#define FRAME_HEIGHT    480

//you need to determine the k value erick
//EA: this k value determines the speed of the servo
#define K 0.30f

#define BUZZER_PIN     21          // pick whatever pin you wire it to
#define LASER_PIN 22  

#define TILT_PIN        0 //vertical movement
#define PAN_PIN         1 //horizontal movement

// ---- Servo pulse + mechanical safety limits -------------------------------
#define SERVO_MIN_US    500u 
#define SERVO_MAX_US    2500u
#define SERVO_FRAME_US  20000u

#define PAN_MIN_DEG    10 //this prevents the servo from locking
#define PAN_MAX_DEG    170
#define TILT_MIN_DEG   20
#define TILT_MAX_DEG   150

//EA: add servo write funct here
static void servo_write(uint gpio, int angle_deg) {
    if (angle_deg < 0)   angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;
    uint span = SERVO_MAX_US - SERVO_MIN_US;
    uint pulse_us = SERVO_MIN_US + (span * (uint)angle_deg) / 180u;
    pwm_set_gpio_level(gpio, pulse_us);
}

//EA:  these track where the servos are pointing right now.
static int  pan_angle      = 90;
static int  tilt_angle     = 90;
static bool prev_detected  = false;
static absolute_time_t buzzer_off_time;
static bool buzzer_active  = false;

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

            //EA: Rising-edge beep: only trigger when detection just BECAME true.
            if (!prev_detected) {
                gpio_put(BUZZER_PIN, 1);
                buzzer_off_time = make_timeout_time_ms(80);   // ~80 ms chirp
                buzzer_active = true;
            }

            //now, we need to calculate the peripherals error offset (from the servo)
            //EA: 1) Pixel error from frame center.
            float error_x = (float)centroid.centroid_x - (FRAME_WIDTH / 2.0f);
            float error_y = (float)centroid.centroid_y - (FRAME_HEIGHT/2.0f);

            //FOR ERICK: put the code for adjusting the angles of the pan and tilt here
            //the equations are in the checklist portio of the packet
            //EA: 2) Proportional correction to current angles.
            //EA: Flip the sign on either line if that axis runs the wrong way.
             pan_angle  = 90 - (int)(K * error_x);
            tilt_angle = 90 + (int)(K * error_y);
            
            //FOR ERICK: You can put your clamp code here as well :D
            //EA: 3) Clamp to the bracket's safe range.
            if (pan_angle  < PAN_MIN_DEG)  pan_angle  = PAN_MIN_DEG;
            if (pan_angle  > PAN_MAX_DEG)  pan_angle  = PAN_MAX_DEG;
            if (tilt_angle < TILT_MIN_DEG) tilt_angle = TILT_MIN_DEG;
            if (tilt_angle > TILT_MAX_DEG) tilt_angle = TILT_MAX_DEG;

            //FOR ERICK: you can put your servo_write code with the adjusted pan and tilt angles here if ya want
            // 4) Command the servos.
            //ea: Any path can be programmed such as side to side or in a square path.
            servo_write(PAN_PIN,  pan_angle);
            servo_write(TILT_PIN, tilt_angle);

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
    prev_detected = centroid.color_detected;

    // Non-blocking buzzer auto-off: don't use sleep_ms here, it would freeze the tick.
    if (buzzer_active && absolute_time_diff_us(get_absolute_time(), buzzer_off_time) <= 0) {
    gpio_put(BUZZER_PIN, 0);
    buzzer_active = false;
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
    //EA: code move into tick function. Might need to add time counter to wait, instead of sleep, for the servo to move.
        servo_write(PAN_PIN, 90);
        servo_write(TILT_PIN, 90);
        sleep_ms(1000);

    //laser
    gpio_init(LASER_PIN);
    gpio_set_dir(LASER_PIN, GPIO_OUT);
    gpio_put(LASER_PIN, 0);
    //buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    //initialize main timer
    struct repeating_timer main_timer;
    add_repeating_timer_ms(-33, main_TICK, NULL, &main_timer);

    //keep main alive so that our camera and main timer keep running
    while(true){}

}
