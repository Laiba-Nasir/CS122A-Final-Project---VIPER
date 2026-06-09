// NOTE: submodules are supplied on the synth/sim command line by the
// Makefile (the SRC file list), so no `include directives are needed here.
// Including them again would double-declare every module.

module top(
    input logic CLK,
    input logic LCD_PCLK,
    input logic VSYNC,
    input logic HSYNC, 
    input logic [7:0] DE,

    output logic LCD_CLK,
    output logic LCD_DEN,
    output logic [4:0] LCD_R,
    output logic [5:0] LCD_G,
    output logic [4:0] LCD_B,
    output logic XCLK
);

//i will be putting the lcd code on the top.sv
assign LCD_CLK = CLK;

// No external reset pin on the board: tie reset low (matches working build).
wire rst = 1'b0;

// we need to declare the internal signals
logic [15:0] raw_color; //this will store the raw color data from our camera
logic valid_color;  // stores the valid raw color
logic frame_valid;  // will store the valid frame signal from our camera

logic [15:0] processed_color;   // store our processed color 
logic processed_valid;  // store the valid signal
logic [9:0] cam_x;   // driven by pixel_capture
logic [8:0] cam_y;   // driven by pixel_capture

logic [9:0] lcd_x;
logic [9:0] lcd_y;
logic [7:0] lcd_color;   // RGB332 read back from the framebuffer

//instantiation of phase 1 (camera) will go here
// =========================================================
// XCLK Generator — feed 25 MHz clock to camera
// =========================================================
xclk_gen u_xclk_gen (
    .CLK(CLK),
    .cam_xclk(XCLK)
);

// =========================================================
// Pixel Capture — grab bytes from camera, assemble RGB565
// =========================================================
pixel_capture u_pixel_capture (
    .pclk(LCD_PCLK),
    .vsync(VSYNC),
    .href(HSYNC),
    .data(DE),
    .pixel_rgb565(processed_color),
    .pixel_valid(processed_valid),
    .pixel_x(cam_x),
    .pixel_y(cam_y),
    .frame_done(frame_valid)
);


//instantiation of phase 3 (color detection) will go here

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
wire cropped = x_cropped && y_cropped && processed_valid; // we only want to write to our line buffer if the pixel is cropped and valid

// =========================================================
// DOUBLE-BUFFERED frame buffer  (kills the dot/line artifacts)
// ---------------------------------------------------------
// Keep TWO frames. The camera writes one buffer while the LCD reads the
// other, and we swap at frame boundaries so the buffer being displayed is
// never being written -> no collision sparkle, no tear.
//
// NOTE: triple-buffering was tried (fully removes the rate-mismatch
// residual in simulation) but looked worse on the real panel, so we keep
// the simpler 2-buffer scheme by choice.
//
// Memory: keep the 2x downsample (240x136) AND store 8-bit RGB332. Two
// buffers = 2 * 240 * 136 * 8b = ~510 Kbit (~32 of 56 EBR) -> fits. RGB565
// is packed to RGB332 on write and expanded back on read; the detection
// pipeline can still tap full RGB565 straight from pixel_capture.
// =========================================================
localparam FB_W   = 240;            // downsampled frame width  (480 / 2)
localparam FB_H   = 136;            // downsampled frame height (272 / 2)
localparam FB_PIX = FB_W * FB_H;    // 32640 words per buffer

// position inside the cropped 480x272 window (valid only when `cropped`)
wire [9:0] crop_x = cam_x - 80;   // 0..479
wire [9:0] crop_y = cam_y - 104;  // 0..271

// within-frame addresses (0 .. FB_PIX-1)
wire [15:0] waddr_base = ((crop_y >> 1) * FB_W) + (crop_x >> 1); // write: 2x downsample
// read: 2x upscale. Use (lcd_x + 1) so the address is presented one LCD
// clock early, compensating the 1-cycle registered-read latency in
// line_buffer (otherwise the picture is shifted right by one column).
wire [15:0] raddr_base = ((lcd_y  >> 1) * FB_W) + ((lcd_x + 1) >> 1);

// pack RGB565 -> RGB332 for storage (keep the top bits of each channel)
wire [7:0] fb_wdata = {processed_color[15:13],   // R: 3 MSBs
                       processed_color[10:8],     // G: 3 MSBs
                       processed_color[4:3]};     // B: 2 MSBs

// ---- buffer-swap control ----------------------------------------------
// Camera (LCD_PCLK) domain: wbuf = the buffer currently being WRITTEN.
// Toggle it at each frame start (frame_valid rising edge) so the
// just-finished frame becomes the one available for display.
logic       wbuf = 1'b0;
logic       frame_valid_d = 1'b0;
always_ff @(posedge LCD_PCLK) begin
    frame_valid_d <= frame_valid;
    if (frame_valid && !frame_valid_d)
        wbuf <= ~wbuf;
end

// LCD (LCD_CLK) domain: bring wbuf across with a 2-FF synchronizer, then
// latch the display buffer ONCE per LCD frame (at the top-left pixel) so
// it can never swap mid-frame -> no tearing.
logic [1:0] wbuf_sync = 2'b00;
logic       rbuf = 1'b0;
always_ff @(posedge LCD_CLK) begin
    wbuf_sync <= {wbuf_sync[0], wbuf};
    if (lcd_x == 0 && lcd_y == 0)
        rbuf <= ~wbuf_sync[1];        // read the buffer the camera is NOT writing
end

// final addresses: high half of the array = buffer 1, low half = buffer 0
wire [15:0] fb_waddr = wbuf ? (waddr_base + FB_PIX) : waddr_base;
wire [15:0] fb_raddr = rbuf ? (raddr_base + FB_PIX) : raddr_base;

//instantiate frame buffer (double-buffered, dual-clock, 8-bit RGB332)
line_buffer #(
    .WIDTH(8),
    .NUM_PIXELS(FB_W),
    .LINES(2*FB_H)            // two frames back-to-back
) line_buffer_inst (
    .cam_clk(LCD_PCLK),
    .lcd_clk(LCD_CLK),
    .we(cropped),
    .waddr(fb_waddr),
    .wdata(fb_wdata),
    .raddr(fb_raddr),
    .rst(rst),
    .rdata(lcd_color)
);

//instantiate lcd_timing
lcd_timing lcd_timing_inst(
    .CLK(LCD_CLK),
    .rst(rst),
    .x_cnt(lcd_x),
    .y_cnt(lcd_y),
    .LCD_DEN(LCD_DEN)
);

//The rest of the code for the lcd, in top.sv, will go here
//unpack stored RGB332 -> LCD pins (replicate the MSBs to keep brightness)
always @(*) begin
    if (LCD_DEN) begin
        LCD_R = {lcd_color[7:5], lcd_color[7:6]};                 // R 3b -> 5b
        LCD_G = {lcd_color[4:2], lcd_color[4:2]};                 // G 3b -> 6b
        LCD_B = {lcd_color[1:0], lcd_color[1:0], lcd_color[1]};   // B 2b -> 5b
    end else begin
        LCD_R = 0;
        LCD_G = 0;
        LCD_B = 0;
    end
end

endmodule