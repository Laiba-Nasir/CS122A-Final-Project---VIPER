module xclk_gen (
    input CLK, // fpga_board_clk
    output cam_xclk  //camera_clk
);

assign cam_xclk = CLK; 
         
endmodule
