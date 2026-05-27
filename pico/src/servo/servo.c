// PWM + proportional control
// dual_servo_pico.c
// Dual-axis (pan/tilt) servo control for the Raspberry Pi Pico 2 W using the
// official Pico SDK. Drives two SG90-class 9g servos (the XiaoR PTZ kit) with
// the RP2350 hardware PWM block.
//
// Concept: we clock the PWM counter at 1 tick = 1 microsecond, so a channel
// "level" is just the pulse width in microseconds (1500 = center). The 20,000
// tick wrap gives a 20 ms frame = 50 Hz, the standard servo frame rate.
//
// CMakeLists.txt: add  hardware_pwm  and  hardware_clocks  to target_link_libraries.
 
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
 
// ---- Servo pulse calibration -----------------------------------------------
// SG90-class: ~500-2500 us spans roughly 0-180 deg. Datasheet nominal is
// 1000-2000 us; widen toward 500/2500 for fuller travel, but tune to YOUR
// servo and back off if it buzzes or jams at the extremes.
#define SERVO_MIN_US    500u     // pulse at 0 degrees
#define SERVO_MAX_US    2500u    // pulse at 180 degrees
#define SERVO_FRAME_US  20000u   // 20 ms => 50 Hz
 
// ---- Pan/tilt mechanical safety limits (degrees) ---------------------------
// The bracket hits hard stops before a full 0-180 sweep. Keep inside these so
// the servo never stalls against the frame. Adjust to your assembled rig.
#define PAN_MIN_DEG     10
#define PAN_MAX_DEG     170
#define TILT_MIN_DEG    20
#define TILT_MAX_DEG    150
 
// ---- Pin assignment --------------------------------------------------------
// GP0 and GP1 share PWM slice 0 (channels A and B). Same 50 Hz frame, fully
// independent pulse widths -- ideal for a 2-servo rig. Do NOT pair two pins on
// the SAME channel (e.g. GP0 and GP16), or they'd move in lockstep.
#define PAN_PIN   0   // GP0 -> slice 0, channel A
#define TILT_PIN  1   // GP1 -> slice 0, channel B
 
// Configure one GPIO as a 50 Hz servo PWM output.
static void servo_init(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
 
    // 1 tick = 1 us, derived from the live system clock so this is correct on
    // both the 125 MHz Pico and the 150 MHz Pico 2 (and any overclock).
    float div = (float)clock_get_hz(clk_sys) / 1000000.0f;
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, SERVO_FRAME_US - 1);   // 20,000 ticks = 20 ms frame
    pwm_set_enabled(slice, true);
}
 
// Command a servo on `gpio` to an angle in [0, 180] degrees.
static void servo_write(uint gpio, int angle_deg) {
    if (angle_deg < 0)   angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;
    uint span = SERVO_MAX_US - SERVO_MIN_US;
    uint pulse_us = SERVO_MIN_US + (span * (uint)angle_deg) / 180u;
    pwm_set_gpio_level(gpio, pulse_us);   // level is in us (1 tick = 1 us) //ea: seems to be from one of the libraries.
}
 
static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
 
int main(void) {
    stdio_init_all();
 
    servo_init(PAN_PIN);
    servo_init(TILT_PIN);
 
    // Center both axes, then pause to let them settle.
    servo_write(PAN_PIN, 90);
    servo_write(TILT_PIN, 90);
    sleep_ms(500);
 
    // Proof-of-life: sweep pan across its safe range, tilt tracks alongside
    // (clamped to its own limits). Replace this loop with your control logic.
    while (true) {
        for (int a = PAN_MIN_DEG; a <= PAN_MAX_DEG; a += 2) {
            servo_write(PAN_PIN,  a);
            servo_write(TILT_PIN, clampi(a, TILT_MIN_DEG, TILT_MAX_DEG));
            sleep_ms(20);
        }
        for (int a = PAN_MAX_DEG; a >= PAN_MIN_DEG; a -= 2) {
            servo_write(PAN_PIN,  a);
            servo_write(TILT_PIN, clampi(a, TILT_MIN_DEG, TILT_MAX_DEG));
            sleep_ms(20);
        }
    }
}
 