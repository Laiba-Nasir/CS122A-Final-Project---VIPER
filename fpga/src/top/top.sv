`include "display/line_buffer.sv"
`include "display/lcd_timing.sv"

module top(
    input logic CLK,
    input logic rst,

    output logic LCD_CLK,
    output logic LCD_DEN,
    output logic [4:0] LCD_R,
    output logic [5:0] LCD_G,
    output logic [4:0] LCD_B
);

//i will be putting the lcd code on the top.sv
assign LCD_CLK = CLK;

logic [9:0] lcd_x;
logic [9:0] lcd_y;
logic [15:0] lcd_color;

//we need to create a mock camera input for testing purposes
logic [9:0] cam_x = 0;
logic [9:0] cam_y = 0;
logic [15:0] cam_color = 0;

//our camera has a 640 x 480 output, so first we need to simulate that
always_ff @(posedge CLK) begin
    if (rst) begin
        cam_x <= 0;
        cam_y <= 0;
        cam_color <= 0;
    end else begin
        if(cam_x < 639) begin
            cam_x <= cam_x + 1;
        end else begin
            mock_x <= 0;
            if(cam_y < 479) begin
                cam_y <= cam_y + 1;
            end else begin
                cam_y <= 0;
            end
        end
    end

    //test RGB

end

//now, we need to do camera cropping and some math for addressing the line buffer
//we know that out active range is 480 x 272 for our LCD while our camera has a 640 x 480 output\

//first we need to crop our camera input so that it can fit on our LCD
/*
    since we have 620 and 480 pixels, we subtract and divide them by 2 to get the 
    number of pixels we need to crop on each side. So, we need to crop 80 pixels on each side for x.
    for y, you do the same with 480 and 272, which gives us 104.
*/
wire x_cropped = (cam_x >= 80) && (cam_x < 560); // we get 560 by adding our cropped pixels withg our active horizontal range
wire y_cropped = (cam_y >= 104) && (cam_y < 376); // we get 376 by adding our cropped pixels withg our active vertical range
wire cropped = x_cropped && y_cropped;

//we need to create a write and read address for our line buffer
wire [9:0] cropped_waddr = (cam_y - 104) * 480 + (cam_x - 80); // we subtract to get the correct address from cropped val
wire [9:0] cropped_raddr = (lcd_y - 104) * 480 + (lcd_x - 80); 

//instantiate line buffer
line_buffer line_buffer_inst #(
    .WIDTH(16),
    .NUM_PIXELS(480),
    .LINES(2)
)(
    .cam_clk(CLK),
    .lcd_clk(CLK),
    .we(cropped),
    .waddr(cropped_waddr),
    .wdata(cam_color),
    .raddr(cropped_raddr),
    .rst(rst),
    .rdata(lcd_color)
);

//instantiate lcd_timing
lcd_timing lcd_timing_inst(
    .clk(CLK),
    .rst(rst),
    .x_cnt(lcd_x),
    .y_cnt(lcd_y),
    .LCD_DEN(LCD_DEN)
);

//The rest of the code for the lcd, in top.sv, will go here
//such as the color pin mapping 
always @(*) begin
    if (LCD_DEN) begin
        LCD_R = lcd_color[15:11];
        LCD_G = lcd_color[10:5];
        LCD_B = lcd_color[4:0];
    end else begin
        LCD_R = 0;
        LCD_G = 0;
        LCD_B = 0;
    end
end

endmodule