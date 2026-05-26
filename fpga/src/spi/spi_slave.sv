//this will be our SPI slave module. 

//in our main loop, the FPGA needs to poll over the SPI to read control coordinates
//holds centroid registers (centroid_x, centroid_y, and target_found)
//shifts bytes out on MISO when PICO requests

module spi_slave(
    input  CLK, 
    input SPI_SCK,
    input SPI_MISO,
    input SPI_CS,

    output reg [7:0] centroid_x,
    output reg [7:0] centroid_y,
    output reg target_found
);

endmodule