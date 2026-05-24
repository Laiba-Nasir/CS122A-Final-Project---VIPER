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

//instantiate line buffer
line_buffer line_buffer_inst #(
    .WIDTH(16),
    .NUM_PIXELS(480),
    .LINES(2)
)(
    .cam_clk(CLK),
    .lcd_clk(CLK),
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

endmodule