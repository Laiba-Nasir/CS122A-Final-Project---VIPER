module line_buffer #(
    parameter WIDTH = 16,
    parameter NUM_PIXELS = 480,
    parameter LINES = 2,
    parameter ADDR_WIDTH = $clog2(NUM_PIXELS * LINES)
)(
    //inputs
    //remember, we need to seperate our read and write clocks
    //this is because the LCD and camera have different frequences
    input logic cam_clk, 
    input logic lcd_clk,
    input logic we,
    input logic [ADDR_WIDTH-1:0] waddr,
    input logic [WIDTH-1:0] wdata,
    input logic [ADDR_WIDTH-1:0] raddr,

    input logic rst, 

    //outputs
    output logic [WIDTH-1:0] rdata
);

//line buffer code will go here
//first, we need an array for the memory
logic [WIDTH-1:0] line_mem [0:NUM_PIXELS*LINES-1];

//always loops for writing(cam) and reading(lcd)
always_ff @(posedge cam_clk) begin
    if (we) begin
        line_mem[waddr] <= wdata;
    end 
end

always_ff @(posedge lcd_clk) begin
    if (rst) begin
        rdata <= 0;
    end else begin
        rdata <= line_mem[raddr];
    end
end


endmodule