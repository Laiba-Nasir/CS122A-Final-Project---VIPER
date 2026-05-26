module top (
    // Board clock
    input CLK,              // 25 MHz board oscillator (P6)

    // Camera interface
    output cam_xclk,        // XCLK output to camera (A2)
    input cam_pclk,         // PCLK from camera (A4)
    input cam_vsync,        // VSYNC from camera (G2)
    input cam_href,         // HREF from camera (K1)
    input [7:0] cam_data,   // D[7:0] from camera (A5-B8)

    // Debug LEDs
    output led_green,       // Green LED (A11) — blinks with frame_done
    output led_red          // Red LED (B11) — glows when pixels flowing
);

    // =========================================================
    // Internal wires — connect modules together
    // =========================================================
    wire [15:0] pixel_rgb565;
    wire pixel_valid;
    wire [9:0] pixel_x;
    wire [8:0] pixel_y;
    wire frame_done;

    // =========================================================
    // XCLK Generator — feed 25 MHz clock to camera
    // =========================================================
    xclk_gen u_xclk_gen (
        .CLK(CLK),
        .cam_xclk(cam_xclk)
    );

    // =========================================================
    // Pixel Capture — grab bytes from camera, assemble RGB565
    // =========================================================
    pixel_capture u_pixel_capture (
        .pclk(cam_pclk),
        .vsync(cam_vsync),
        .href(cam_href),
        .data(cam_data),
        .pixel_rgb565(pixel_rgb565),
        .pixel_valid(pixel_valid),
        .pixel_x(pixel_x),
        .pixel_y(pixel_y),
        .frame_done(frame_done)
    );

    // =========================================================
    // Debug LED logic
    // =========================================================

    // Green LED: toggle on every frame_done pulse
    // If camera is sending ~30fps, this toggles at ~15Hz (visible blink)
    reg green_toggle;
    always @(posedge cam_pclk) begin
        if (frame_done)
            green_toggle <= ~green_toggle;
    end
    assign led_green = green_toggle;

    // Red LED: stays on while pixel_valid is firing
    // At 30fps with 640x480, pixel_valid is high ~50% of the time
    // so this will look like a steady glow if pixels are flowing
    assign led_red = pixel_valid;

endmodule