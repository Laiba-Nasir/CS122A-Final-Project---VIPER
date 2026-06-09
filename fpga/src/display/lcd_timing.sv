module lcd_timing #(
    parameter active_x = 480,
    parameter totFrame_x = 526,
    parameter active_y = 272,
    parameter totFrame_y = 286
)(
    //inputs
    input CLK,
    input rst,

    //outputs
    output reg [9:0] x_cnt = 0,
    output reg [9:0] y_cnt = 0,
    output LCD_DEN
);
    //Some of the lcd code from lab 6 will go here
    // NOTE - this is tested max, dont change timing furhter 
    always @(posedge CLK) begin
        if(rst) begin
            x_cnt <= 0;
            y_cnt <= 0;
        end else begin
            if (x_cnt < totFrame_x - 1) begin
                x_cnt <= x_cnt + 1;
            end else begin
                x_cnt <= 0;
                if(y_cnt < totFrame_y - 1) begin
                    y_cnt <= y_cnt + 1;
                end else begin
                    y_cnt <= 0;
                end
            end
        end
    end

    //ddisplay enable [DE] - only high when active 
    assign LCD_DEN = (x_cnt < active_x) && (y_cnt < active_y);

endmodule