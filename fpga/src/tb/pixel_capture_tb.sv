`timescale 1ns / 1ps

module pixel_capture_tb;

    reg pclk;
    reg vsync;
    reg href;
    reg [7:0] data;

    wire [15:0] pixel_rgb565;
    wire pixel_valid;
    wire [9:0] pixel_x;
    wire [8:0] pixel_y;
    wire frame_done;

    pixel_capture uut (
        .pclk(pclk),
        .vsync(vsync),
        .href(href),
        .data(data),
        .pixel_rgb565(pixel_rgb565),
        .pixel_valid(pixel_valid),
        .pixel_x(pixel_x),
        .pixel_y(pixel_y),
        .frame_done(frame_done)
    );

    initial pclk = 0;
    always #20 pclk = ~pclk;

    task send_row;
        input [15:0] num_pixels;
        integer i;
        begin
            @(negedge pclk);
            href = 1;
            data = 8'h00;

            for (i = 0; i < num_pixels; i = i + 1) begin
                @(negedge pclk);
                data = i[7:0] + 8'h80;

                @(negedge pclk);
                if (i < num_pixels - 1)
                    data = (i + 1);
                else
                    data = 8'h00;
            end

            @(negedge pclk);
            href = 0;
            data = 8'h00;

            repeat(10) @(posedge pclk);
        end
    endtask

    initial begin
        $dumpfile("pixel_capture_tb.vcd");
        $dumpvars(0, pixel_capture_tb);

        vsync = 0;
        href = 0;
        data = 8'h00;

        repeat(5) @(posedge pclk);

        $display("--- TEST 1: VSYNC pulse ---");
        @(negedge pclk);
        vsync = 1;
        @(posedge pclk); #1;
        @(negedge pclk);
        vsync = 0;

        repeat(5) @(posedge pclk);

        $display("--- TEST 2: Sending 3 rows of 5 pixels ---");

        $display("  Row 0:");
        send_row(5);

        $display("  Row 1:");
        send_row(5);

        $display("  Row 2:");
        send_row(5);

        $display("--- TEST 3: Second VSYNC pulse ---");
        @(negedge pclk);
        vsync = 1;
        @(posedge pclk); #1;
        @(negedge pclk);
        vsync = 0;

        repeat(5) @(posedge pclk);

        $display("  Row 0 after reset:");
        send_row(3);

        repeat(10) @(posedge pclk);
        $display("--- TESTBENCH COMPLETE ---");
        $finish;
    end

    // Monitor on negedge — values have settled
    always @(negedge pclk) begin
        if (pixel_valid)
            $display("  PIXEL: x=%0d y=%0d rgb565=0x%04h", pixel_x, pixel_y, pixel_rgb565);
        if (frame_done)
            $display("  FRAME_DONE pulse detected!");
    end

endmodule