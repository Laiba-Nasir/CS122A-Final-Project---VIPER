module lcd_timing #(
    parameter active_x = 480,
    parameter toFrame_x = 526,
    parameter active_y = 272,
    parameter totFrame_y = 286,
)(
    //inputs
    input clk,
    input rst,

    //outputs
    output reg [9:0] x_cnt,
    output reg [9:0] y_cnt,
);

    always_ff @(posedge CLK) begin
        if(rst) begin
            x_cnt <= 0;
            y_cnt <= 0;
        end else if (x_cnt < totFrame_X - 1) begin
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

endmodule