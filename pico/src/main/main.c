// // SPI, servos, laser, buzzer


// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/i2c.h"

// // OV7670 I2C address (7-bit: 0x21, write byte: 0x42)
// #define OV7670_ADDR 0x21

// // I2C pins — must match your wiring
// #define SDA_PIN 0  // GP0
// #define SCL_PIN 1  // GP1

// // Write one register to the OV7670 over I2C (SCCB)
// void ov7670_write_reg(uint8_t reg, uint8_t val) {
//     uint8_t buf[2] = {reg, val};
//     i2c_write_blocking(i2c0, OV7670_ADDR, buf, 2, false);
//     sleep_ms(1); // small delay between writes — OV7670 needs this
// }

// // Read one register from the OV7670
// uint8_t ov7670_read_reg(uint8_t reg) {
//     uint8_t val;
//     i2c_write_blocking(i2c0, OV7670_ADDR, &reg, 1, true);
//     i2c_read_blocking(i2c0, OV7670_ADDR, &val, 1, false);
//     return val;
// }

// // Configure the OV7670 for RGB565 output
// void ov7670_init(void) {
//     // Reset all registers to default
//     ov7670_write_reg(0x12, 0x80);
//     sleep_ms(100); // wait for reset to complete

//     // Core format settings
//     ov7670_write_reg(0x12, 0x04); // COM7: RGB output
//     ov7670_write_reg(0x40, 0xD0); // COM15: RGB565, full output range
//     ov7670_write_reg(0x11, 0x01); // CLKRC: internal clock prescaler /2
//     ov7670_write_reg(0x0C, 0x00); // COM3: no scaling
//     ov7670_write_reg(0x3E, 0x00); // COM14: no PCLK scaling

//     // VGA window settings (default 640x480)
//     ov7670_write_reg(0x17, 0x13); // HSTART
//     ov7670_write_reg(0x18, 0x01); // HSTOP
//     ov7670_write_reg(0x19, 0x02); // VSTRT
//     ov7670_write_reg(0x1A, 0x7A); // VSTOP
//     ov7670_write_reg(0x32, 0xB6); // HREF
//     ov7670_write_reg(0x03, 0x0A); // VREF

//     // Color matrix and gamma (known-good values from AngeloJacobo)
//     ov7670_write_reg(0x4F, 0x80); // MTX1
//     ov7670_write_reg(0x50, 0x80); // MTX2
//     ov7670_write_reg(0x51, 0x00); // MTX3
//     ov7670_write_reg(0x52, 0x22); // MTX4
//     ov7670_write_reg(0x53, 0x5E); // MTX5
//     ov7670_write_reg(0x54, 0x80); // MTX6
//     ov7670_write_reg(0x58, 0x9E); // MTXS

//     // Automatic controls
//     ov7670_write_reg(0x13, 0xE7); // COM8: enable AGC, AWB, AEC
//     ov7670_write_reg(0x14, 0x48); // COM9: 16x AGC ceiling
//     ov7670_write_reg(0x15, 0x00); // COM10: normal VSYNC/HREF/PCLK

//     // Gamma curve
//     ov7670_write_reg(0x7A, 0x20); // SLOP
//     ov7670_write_reg(0x7B, 0x10); // GAM1
//     ov7670_write_reg(0x7C, 0x1E); // GAM2
//     ov7670_write_reg(0x7D, 0x35); // GAM3
//     ov7670_write_reg(0x7E, 0x5A); // GAM4
//     ov7670_write_reg(0x7F, 0x69); // GAM5
//     ov7670_write_reg(0x80, 0x76); // GAM6
//     ov7670_write_reg(0x81, 0x80); // GAM7
//     ov7670_write_reg(0x82, 0x88); // GAM8
//     ov7670_write_reg(0x83, 0x8F); // GAM9
//     ov7670_write_reg(0x84, 0x96); // GAM10
//     ov7670_write_reg(0x85, 0xA3); // GAM11
//     ov7670_write_reg(0x86, 0xAF); // GAM12
//     ov7670_write_reg(0x87, 0xC4); // GAM13
//     ov7670_write_reg(0x88, 0xD7); // GAM14
//     ov7670_write_reg(0x89, 0xE8); // GAM15
// }

// int main() {
//     // Init Pico standard library
//     stdio_init_all();

//     // Init I2C0 at 100kHz (OV7670 SCCB max is 400kHz, 100k is safer)
//     i2c_init(i2c0, 100 * 1000);

//     // Set up I2C pins
//     gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
//     gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

//     // Enable internal pull-ups for I2C lines
//     gpio_pull_up(SDA_PIN);
//     gpio_pull_up(SCL_PIN);

//     // Wait a bit for camera to power up and XCLK to stabilize
//     sleep_ms(100);

//     // Configure the camera for RGB565 output
//     ov7670_init();

//     // Verify camera is responding by reading product ID
//     uint8_t pid = ov7670_read_reg(0x0A); // should return 0x76
//     uint8_t ver = ov7670_read_reg(0x0B); // should return 0x73
//     printf("OV7670 PID: 0x%02X, VER: 0x%02X\n", pid, ver);

//     if (pid == 0x76 && ver == 0x73) {
//         printf("Camera detected and configured for RGB565!\n");
//     } else {
//         printf("WARNING: Camera not detected! Check wiring.\n");
//     }

//     // Main loop — nothing to do here for now
//     // Later: SPI polling for centroid, servo control, laser toggle
//     while (true) {
//         sleep_ms(1000);
//     }

//     return 0;
// }