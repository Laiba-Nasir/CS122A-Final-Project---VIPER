module pixel_capture (
    // inputs 
    input vsync, 
    input href, 
    input pclk, 
    input [7:0] data,

    // outputs
    output reg [15:0] pixel_rgb565, 
    output reg pixel_valid, 
    output reg [9:0] pixel_x, 
    output reg [8:0] pixel_y, 
    output reg frame_done
);

// internal 
reg byte_flag; // 0=1st byte, 1=2nd byte
reg [7:0] temp_byte; // holds 1st byte while waiting for 2nd 
reg prev_vsync; // need this since we need to detect the rising edge of the vsync 
reg [9:0] pixel_count; // counts completed pixels, pixel_x is set from this

always @(posedge pclk) begin

    // when VSYNC rising edge, starting a new frame so reset everything 
    if (vsync == 1 && prev_vsync == 0) begin
        pixel_count <= 0; 
        pixel_x <= 0;
        pixel_y <= 0; 
        byte_flag <= 0; 
        frame_done <= 1; 

    // when href is high, the data bus has real pixel data
    end else if (href == 1) begin
        frame_done <= 0;

        // if its the 1st byte, byte_flag = 0
        if (byte_flag == 0) begin
            temp_byte <= data[7:0]; 
            byte_flag <= 1; 
            pixel_valid <= 0;
            
        end else begin
            pixel_rgb565 <= {temp_byte, data}; 
            pixel_valid <= 1; 
            byte_flag <= 0; 
            pixel_x <= pixel_count;      // output current position
            pixel_count <= pixel_count + 1; // increment for next pixel
            
        end 
        
    // if HREF is low, its a gap between rows 
    end else begin
        frame_done <= 0;
        pixel_valid <= 0; 
        byte_flag <= 0; 
        if (pixel_count > 0) begin
            pixel_y <= pixel_y + 1; 
            pixel_count <= 0;
            pixel_x <= 0; 
        end
    end

    prev_vsync <= vsync;
    
end
         
endmodule