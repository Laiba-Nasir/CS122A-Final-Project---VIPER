//this will be our SPI slave module. 

//in our main loop, the FPGA needs to poll over the SPI to read control coordinates
//holds centroid registers (centroid_x, centroid_y, and target_found)
//shifts bytes out on MISO when PICO requests

module spi_slave(
    input  CLK, 
    input SPI_SCK,
    input SPI_MOSI,
    input SPI_CS,

    output reg SPI_MISO,
    output reg [15:0] centroid_x,
    output reg [15:0] centroid_y,
    output reg target_found
);

//put our registers here
reg [39:0] shift_reg; //40 bits to hold 5 bytes of data (centroid_x, centroid_y, target_found)
reg [5:0] shift_bit; //track how many bits have been shifted 

//SPI slave logic
//we need to the falling edge of the CS so that we can latch onto the data 
always @(negedge SPI_CS) begin
    //we need to latch our data into the shift register
    shift_reg <= {centroid_x[15:8],
                  centroid_x[7:0],
                  centroid_y[15:8],
                  centroid_y[7:0],
                  target_found[0]};
    shift_bit <= 0;
end

//now, we need to shift our data out and into the falling edge of SCK
always @(negedge SPI_SCK) begin
end

endmodule