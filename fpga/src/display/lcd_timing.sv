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
    output reg [9:0] x_cnt,
    output reg [9:0] y_cnt,
    output LCD_DEN
);
    //Some of the lcd code from lab 6 will go here
    
    //the parameters for the horizontal(x) and vertical(y) axis have been given
    /*
        Parameter	            Horizontal	    Vertical
        Active region	        480 pixels	    272 lines
        Buffer Region	        45 clocks	    13 lines
        Total per line/frame	525 clocks	    285 lines
    */
    always @(posedge CLK) begin
        if(rst) begin
            x_cnt <= 0;
            y_cnt <= 0;
        end else begin
            if (x_cnt < totFrame_X - 1) begin
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

    //Display Enable (DE)
    //only high during active. So, we have to make sur ethe axis cnt is less than the active region.
    assign LCD_DEN = (x_cnt < active_x) && (y_cnt < active_y);

endmodule