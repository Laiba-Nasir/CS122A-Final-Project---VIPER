// PWM + proportional control

// ALL CODE FOR LASER, BUZZER, and SERVO SETUP/CONFIG 
//VERY IMPORTANT FILE
//This is the code that was controlling the servos,lazer,buzzer during the final demo.
//Claude assisted with some setting up and debugging in this file. This is based on pico/src/main.c to help test
// the servo,laser, buzzer independent from the camera, fpga. 

// servo_test.c
// Standalone testbench for the pan/tilt + laser + buzzer pipeline.
// Simulates a fake target that appears on the LEFT side of the frame, sweeps
// across to the RIGHT over 3 seconds, then disappears for 1 second, then
// repeats. Lets you verify the tracking loop, laser, and buzzer without the
// camera/FPGA hardware connected.
//
// Needed for this test: CMakeLists.txt: target_link_libraries(<target> pico_stdlib hardware_pwm hardware_clocks)
    /* put in CMakeLists.txt
    target_link_libraries(testing122FinalProject 
        pico_stdlib
        hardware_pwm
        hardware_clocks
        hardware_timer
        )
    */

#include "pico/stdlib.h" //Using pico libraries.
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

// ---- Frame & control constants --------------------------------------------
//These help calculate where the servo should move if the screen changes size.
#define FRAME_WIDTH     640
#define FRAME_HEIGHT    480
#define K               0.30f //this is servo speed.

// ---- Pin assignments ---Pico GPIO pins----------------------------------------------
#define TILT_PIN        0 //uses pwm
#define PAN_PIN         1 //uses pwm
#define LASER_PIN       22 
#define BUZZER_PIN      21

// ---- Servo pulse + mechanical safety limits -------------------------------
#define SERVO_MIN_US    500u 
#define SERVO_MAX_US    2500u
#define SERVO_FRAME_US  20000u
#define PAN_MIN_DEG     10
#define PAN_MAX_DEG     170
#define TILT_MIN_DEG    20
#define TILT_MAX_DEG    150

// ---- Mock/fake centroid struct (mimics the real fpga_centroid) -----------------
//This helps simulate what would happen if the blob detection from the camera,fpga worked perfectly.
typedef struct {
    int  centroid_x;
    int  centroid_y;
    bool color_detected; //same thing as "target detected"
} fake_centroid;

static fake_centroid centroid = {0, 0, false};

// ---- Servo helpers  -----------------------------
static void servo_init(uint gpio) { 
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    float div = (float)clock_get_hz(clk_sys) / 1000000.0f;
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, SERVO_FRAME_US - 1);
    pwm_set_enabled(slice, true);
}

static void servo_write(uint gpio, int angle_deg) { //input which pin, which angle.
    if (angle_deg < 0)   angle_deg = 0; //protective
    if (angle_deg > 180) angle_deg = 180; //protective
    uint span = SERVO_MAX_US - SERVO_MIN_US;
    uint pulse_us = SERVO_MIN_US + (span * (uint)angle_deg) / 180u; //pwm value to send to servo.
    pwm_set_gpio_level(gpio, pulse_us);
}

// ---- Track servo angle and buzzer  --------------------------------------------
static int  pan_angle      = 90;
static int  tilt_angle     = 90;
static bool prev_detected  = false;
static absolute_time_t buzzer_off_time;
static bool buzzer_active  = false;

/*
// ---- Simulation state Test 1 -LEFT TO RIGHT---------------------------------------------------
// At ~33 ms per tick:  3 s sweep = ~90 ticks,  1 s gap = ~30 ticks.
#define SIM_SWEEP_TICKS   90    // target visible, moving left -> right
#define SIM_GAP_TICKS     30    // target gone
#define SIM_CYCLE_TICKS   (SIM_SWEEP_TICKS + SIM_GAP_TICKS)

static int sim_tick = 0;

// Stand-in for centroid_read(). Populates the global `centroid` based on the
// simulation timeline and returns true (as if the read succeeded).
static bool fake_centroid_read(void) {
    if (sim_tick < SIM_SWEEP_TICKS) {
        // Linear sweep from x = 0 to x = FRAME_WIDTH over the sweep window.
        centroid.centroid_x = (sim_tick * FRAME_WIDTH) / SIM_SWEEP_TICKS;
        centroid.centroid_y = FRAME_HEIGHT / 2;   // hold vertical at center
        centroid.color_detected = true;
    } else {
        // Gap: pretend no target is visible.
        centroid.color_detected = false;
    }
    sim_tick = (sim_tick + 1) % SIM_CYCLE_TICKS;
    return true;
}
    */

// ---- Simulation state --Test 2 --Square path-------------------------------------------------
// Square path: from center, up 100 -> right 100 -> down 100 -> left 100, repeat.
// At ~33 ms/tick, 45 ticks ≈ 1.5 s per side, so ~6 s for a full square.
#define SIDE_PIXELS    100
#define SIDE_TICKS     45
#define CYCLE_TICKS    (SIDE_TICKS * 4)
#define CENTER_X       (FRAME_WIDTH  / 2)
#define CENTER_Y       (FRAME_HEIGHT / 2)

static int sim_tick = 0;

static bool fake_centroid_read(void) {
    int phase       = (sim_tick / SIDE_TICKS) % 4; //square path states // which side of the square
    int phase_tick  =  sim_tick % SIDE_TICKS;          // how far along this side
    int progress    = (phase_tick * SIDE_PIXELS) / SIDE_TICKS;  // 0..SIDE_PIXELS

    // Remember: image y increases downward, so "up" is y -= ...
    switch (phase) {
        case 0:  // up:    (CENTER_X,            CENTER_Y - progress)
            centroid.centroid_x = CENTER_X;
            centroid.centroid_y = CENTER_Y - progress;
            break;
        case 1:  // right: (CENTER_X + progress, CENTER_Y - SIDE_PIXELS)
            centroid.centroid_x = CENTER_X + progress;
            centroid.centroid_y = CENTER_Y - SIDE_PIXELS;
            break;
        case 2:  // down:  (CENTER_X + SIDE_PIXELS, CENTER_Y - SIDE_PIXELS + progress)
            centroid.centroid_x = CENTER_X + SIDE_PIXELS;
            centroid.centroid_y = CENTER_Y - SIDE_PIXELS + progress;
            break;
        case 3:  // left:  (CENTER_X + SIDE_PIXELS - progress, CENTER_Y)
            centroid.centroid_x = CENTER_X + SIDE_PIXELS - progress;
            centroid.centroid_y = CENTER_Y;
            break;
    }
    centroid.color_detected = true;   // target always visible — laser stays on. Can be changed so that a target goes in and out of view

    sim_tick = (sim_tick + 1) % CYCLE_TICKS;
    return true;
}





// ---- Main tick — identical logic to the real one, just the fake read ------
bool main_TICK(struct repeating_timer *t) {
    if (fake_centroid_read()) {
        if (centroid.color_detected) {
            gpio_put(LASER_PIN, 1);

            // Rising-edge beep: beep once when a target first appears.
            if (!prev_detected) {
                gpio_put(BUZZER_PIN, 1);
                buzzer_off_time = make_timeout_time_ms(80);
                buzzer_active = true;
            }

            //determin target's distance from the center of the screen.
            float error_x = (float)centroid.centroid_x - (FRAME_WIDTH  / 2.0f);
            float error_y = (float)centroid.centroid_y - (FRAME_HEIGHT / 2.0f);

            /*
            pan_angle  -= (int)(K * error_x); //if we use this, the servo gets stuck at an edge.
            tilt_angle += (int)(K * error_y);
            */

            // New direct mapping angle is a function of where the target currently located.
            pan_angle  = 90 - (int)(K * error_x);
            tilt_angle = 90 + (int)(K * error_y);

            //enforce range restriction to prevent servo getting stuck.
            if (pan_angle  < PAN_MIN_DEG)  pan_angle  = PAN_MIN_DEG;
            if (pan_angle  > PAN_MAX_DEG)  pan_angle  = PAN_MAX_DEG;
            if (tilt_angle < TILT_MIN_DEG) tilt_angle = TILT_MIN_DEG;
            if (tilt_angle > TILT_MAX_DEG) tilt_angle = TILT_MAX_DEG;

            //makes the servo move as it should.
            servo_write(PAN_PIN,  pan_angle);
            servo_write(TILT_PIN, tilt_angle);

            printf("[SIM] x=%3d y=%3d  pan=%3d tilt=%3d\n",
                   centroid.centroid_x, centroid.centroid_y,
                   pan_angle, tilt_angle);
        } else {
            //if no target/color blob detected, then turn off the laser.
            gpio_put(LASER_PIN, 0);
            printf("[SIM] no target\n");
        }
    }
    prev_detected = centroid.color_detected;

    // Non-blocking buzzer auto-off. Uses timer to make the buzzer beep very briefly.
    if (buzzer_active && absolute_time_diff_us(get_absolute_time(), buzzer_off_time) <= 0) {
        gpio_put(BUZZER_PIN, 0);
        buzzer_active = false;
    }
    return true;
}

int main(void) {
    stdio_init_all();

    servo_init(PAN_PIN);
    servo_init(TILT_PIN);

    // Center both axes so software state matches physical position.
    servo_write(PAN_PIN, 90);
    servo_write(TILT_PIN, 90);
    sleep_ms(500);

    gpio_init(LASER_PIN);
    gpio_set_dir(LASER_PIN, GPIO_OUT);
    gpio_put(LASER_PIN, 0);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    struct repeating_timer main_timer;
    add_repeating_timer_ms(-33, main_TICK, NULL, &main_timer);

    while (true) { tight_loop_contents(); }
}