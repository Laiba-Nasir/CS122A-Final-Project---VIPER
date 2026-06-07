// I2C write sequence at boot

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ov7670_regs.h"

#include <stdio.h>
#include <stdint.h>

//REMEMBER: to avoid using sleep_ms, we will be doing a statemachine since we need our I2C to start THE MOMENT the camera is powered on. 
//so, rather than using sleep_ms(30), we will using a repeating timer to check whether the camera is ready to recieve data

//global variables
int indx = 0; //we will need to keep track of which register we are writing to next
int ticks = 0; //we need to keep track of how many ticks our program needs to wait before we can start writing to the camera
bool init_ready = false; //we need this so that our main() knows when the program is finished and ready to move on

//states go here
typedef enum {INIT, ON, WRITE, RESET, DONE} state_t;
state_t state = INIT;

//here we will right the registers values (most of these are from the datasheet)
const uint8_t ov7670_init_regs[][2] = {
    
    {OV7670_REG_COM7, 0x80}, //reset the camera
    {OV7670_REG_COM7, 0x14}, //we are putting it into RGB mode
    {OV7670_REG_COM15, 0xD0}, //set it to RGB565 
    {OV7670_REG_COM10, 0x00}, //disable power down and enable normal mode(the default in the datasheet is literally 0x00)
    {OV7670_REG_COM14, 0x00}, //we aren't scaling the image. So, it's not needed
    {OV7670_REG_COM9, 0x48}, //we need to set the gain ceiling to 128x to have better control over the gain
    {OV7670_REG_COM8, 0xE7}, //we should turn on our AGC and AEC
    {OV7670_REG_COM6, 0x41}, 
    {OV7670_REG_COM3, 0x00}, //we don't need to set our scaling
    {OV7670_REG_COM2, 0x03}, //we need a 4x drive capability

    
    {OV7670_REG_RGB444, 0X00}, //turn off rgb444 so that we make sure we are using rgb565
    {OV7670_REG_TSLB, 0x04}, //we need to set the byte order
    {OV7670_REG_CLKRC, 0x00}, //Divide by 1 so that our XCLK is used at 24MHz
    
    {OV7670_REG_HSTART, 0x13}, //default is 0x11 but we are using 0x13 because of RGB565 2-byte format
    {OV7670_REG_HSTOP, 0x01}, 
    {OV7670_REG_HREF, 0xB6},   //we need to make sure we get the correct number of pixels per line
    {OV7670_REG_VSTART, 0x02},
    {OV7670_REG_VSTOP, 0x7A},
    {OV7670_REG_VREF, 0x0A},
    
    {OV7670_REG_MVFP, 0x07}, //we don't need to mirror or flip

    {OV7670_REG_TABLE_END, 0xFF} //this is just a marker to show that we are finished
};

//originally we had all of this in the init function, but for our SM's we need to seperate them into its own function
void ov7670_setup(void){
    i2c_init(i2c0, 100*1000);

    gpio_set_function(4, GPIO_FUNC_I2C);    //this is our SDA pin
    gpio_set_function(5, GPIO_FUNC_I2C);    //this is our SCL pin

    gpio_pull_up(4);
    gpio_pull_up(5);
}

//now, we will need a write function
// we need to write only one register to the camera
void write_reg(uint8_t reg, uint8_t val){
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(i2c0, 0x21, buf, 2, false); //since our SDK expects us to use a 7-bit address, we need to shift our address to the left by 1
}

//we will be writing a read register function
//should write into a register and then read from it
uint8_t read_reg(uint8_t reg){
    uint8_t val;

    i2c_write_blocking(i2c0, 0x21, &reg, 1, true); //make sure to keep the connection alive after writing
    i2c_read_blocking(i2c0, 0x21, &val, 1, false); 

    return val;
}

//SM will go here
//we will be using the timer callback from the lab's to replace our TICK function from CS120B. 
bool Ov7670_TICK(struct repeating_timer *t){
    switch(state){
        case INIT:
            state = ON;
            break;

        case ON:
            //we need to wait 100ms for our Camera to power on 
            if(ticks >= 100){
                ticks = 0; //we need to reset our ticks 
                state = WRITE;
            }else{
                ticks++;
            }
            break;

        case WRITE:
            //We have to check if we are at the end of our register list
            if(ov7670_init_regs[indx][0] == OV7670_REG_TABLE_END){
                state = DONE;
            }
            
            //if we aren't at the end, write to the current register
            write_reg(ov7670_init_regs[indx][0], ov7670_init_regs[indx][1]);

            //now, we need to check if we have written into COM7 with the reset value 0x41
            if(ov7670_init_regs[indx][0] == OV7670_REG_COM7 && ov7670_init_regs[indx][1] == 0x41){ 
                indx++;
                ticks = 0;
                state = RESET;
            }else{
                indx++; //if we haven't written into COM7, tick will move onto the next register
            }
            break;

        case RESET:
            //now, we need to wait 30ms for the camera to reset
            if(ticks >= 30){
                ticks = 0;
                state = WRITE; //go back to write after our reset is done
            }else{
                ticks++;
            }
            break;

        case DONE:
            init_ready = true;
            return false; //we should stop our timer once we are done
            break;

        default:
            break;
    }

    return true;
}

//finally, we will work on our init function
void ov7670_init(){
    return init_true; //we will be calling our add_repeating_timer function in the main code of our pico
}