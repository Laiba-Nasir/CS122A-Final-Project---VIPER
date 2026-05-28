// // I2C write sequence at boot

// // Pico C code — minimum camera config
// i2c_write_reg(0x12, 0x80); // reset all registers
// sleep_ms(100);
// i2c_write_reg(0x12, 0x04); // RGB mode
// i2c_write_reg(0x40, 0xD0); // RGB565, full range
// i2c_write_reg(0x11, 0x01); // clock prescaler