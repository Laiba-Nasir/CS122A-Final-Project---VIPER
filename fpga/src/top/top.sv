

// module top(
//     input logic CLK,
//     // input logic rst,
//     input logic cam_pclk,
//     input logic cam_vsync,
//     input logic cam_href, 
//     input logic [7:0] cam_data, //send to lcdr, gb, convt from 332 -->565

//     output logic LCD_CLK,
//     output logic LCD_DEN,
//     output logic [4:0] LCD_R,
//     output logic [5:0] LCD_G,
//     output logic [4:0] LCD_B,
//     output logic cam_xclk
// );

// //i will be putting the lcd code on the top.sv
// assign LCD_CLK = CLK;

// // we need to declare the internal signals
// logic [15:0] raw_color; //this will store the raw color data from our camera
// logic valid_color;  // stores the valid raw color
// logic frame_valid;  // will store the valid frame signal from our camera

// logic [15:0] processed_color;   // store our processed color 
// logic processed_valid;  // store the valid signal
// logic [9:0] cam_x;
// logic [8:0] cam_y;

// logic [9:0] lcd_x;
// logic [9:0] lcd_y;
// logic [15:0] lcd_color;

// // Remove rst from port list, add this inside the module:
// wire rst = 1'b0;

// //instantiation of phase 1 (camera) will go here
// // =========================================================
// // XCLK Generator — feed 25 MHz clock to camera
// // =========================================================
// xclk_gen u_xclk_gen (
//     .CLK(CLK),
//     .cam_xclk(cam_xclk)
// );

// // =========================================================
// // Pixel Capture — grab bytes from camera, assemble RGB565
// // =========================================================
// pixel_capture u_pixel_capture (
//     .pclk(cam_pclk),
//     .vsync(cam_vsync),
//     .href(cam_href),
//     .data(cam_data),
//     .pixel_rgb565(processed_color),
//     .pixel_valid(processed_valid),
//     .pixel_x(cam_x),
//     .pixel_y(cam_y),
//     .frame_done(frame_valid)
// );


// //instantiation of phase 3 (color detection) will go here

// //now, we need to do camera cropping and some math for addressing the line buffer
// //we know that out active range is 480 x 272 for our LCD while our camera has a 640 x 480 output\

// //first we need to crop our camera input so that it can fit on our LCD
// /*
//     since we have 620 and 480 pixels, we subtract and divide them by 2 to get the 
//     number of pixels we need to crop on each side. So, we need to crop 80 pixels on each side for x.
//     for y, you do the same with 480 and 272, which gives us 104.
// */
// wire x_cropped = (cam_x >= 80) && (cam_x < 560); // we get 560 by adding our cropped pixels withg our active horizontal range
// wire y_cropped = (cam_y >= 104) && (cam_y < 376); // we get 376 by adding our cropped pixels withg our active vertical range
// wire cropped = x_cropped && y_cropped && processed_valid; // we only want to write to our line buffer if the pixel is cropped and valid

// //we need to create a write and read address for our line buffer
// wire [9:0] cropped_waddr = (cam_y[0] * 480) + (cam_x - 80); // we subtract to get the correct address from cropped val
// wire [9:0] cropped_raddr = (~cam_y[0] * 480) +                        ; // we invert cam_y[0] because we want to our cropped raddr to read from the correct line


// //instantiate line buffer
// line_buffer #(
//     .WIDTH(16),
//     .NUM_PIXELS(480),
//     .LINES(2)
// ) line_buffer_inst (
//     .cam_clk(cam_pclk),
//     .lcd_clk(LCD_CLK),
//     .we(cropped),
//     .waddr(cropped_waddr),
//     .wdata(processed_color),
//     .raddr(cropped_raddr),
//     .rst(rst),
//     .rdata(lcd_color)
// );

// //instantiate lcd_timing
// lcd_timing lcd_timing_inst(
//     .CLK(LCD_CLK),
//     .rst(rst),
//     .x_cnt(lcd_x),
//     .y_cnt(lcd_y),
//     .LCD_DEN(LCD_DEN)
// );

// //The rest of the code for the lcd, in top.sv, will go here
// //such as the color pin mapping 
// always @(*) begin
//     if (LCD_DEN) begin
//         LCD_R = lcd_color[15:11];
//         LCD_G = lcd_color[10:5];
//         LCD_B = lcd_color[4:0];
//     end else begin
//         LCD_R = 0;
//         LCD_G = 0;
//         LCD_B = 0;
//     end
// end

// endmodule


module top(
    input logic CLK,
    input logic cam_pclk,
    input logic cam_vsync,
    input logic cam_href, 
    input logic [7:0] cam_data,

    output logic LCD_CLK,
    output logic LCD_DEN,
    output logic [4:0] LCD_R,
    output logic [5:0] LCD_G,
    output logic [4:0] LCD_B,
    output logic cam_xclk
);

wire rst = 1'b0;
assign LCD_CLK = CLK;

logic [9:0] lcd_x;
logic [9:0] lcd_y;

// XCLK — feed clock to camera
xclk_gen u_xclk_gen (
    .CLK(CLK),
    .cam_xclk(cam_xclk)
);

// LCD timing — generates x/y counters and DEN
lcd_timing lcd_timing_inst(
    .CLK(LCD_CLK),
    .rst(rst),
    .x_cnt(lcd_x),
    .y_cnt(lcd_y),
    .LCD_DEN(LCD_DEN)
);

// Raw cam_data straight to LCD as RGB332 → RGB565
always @(*) begin
    if (LCD_DEN) begin
        LCD_R = {cam_data[7:5], cam_data[7:6]};
        LCD_G = {cam_data[4:2], cam_data[4:2]};
        LCD_B = {cam_data[1:0], cam_data[1:0], cam_data[1]};
    end else begin
        LCD_R = 0;
        LCD_G = 0;
        LCD_B = 0;
    end
end

endmodule