`timescale 1ns / 1ps
`include "top/top.sv"

module tb_top;

    // ---------------------------------------------------------
    // 1. Clock and Reset Signals
    // ---------------------------------------------------------
    logic CLK;        // Main FPGA system / LCD clock
    logic rst;
    logic LCD_PCLK;   // Camera pixel clock
    logic VSYNC;
    logic HSYNC;      // Maps to href in pixel capture
    logic [7:0] DE;   // Maps to camera data bus

    // Outputs from DUT
    logic LCD_CLK;
    logic LCD_DEN;
    logic [4:0] LCD_R;
    logic [5:0] LCD_G;
    logic [4:0] LCD_B;
    logic XCLK;

    // ---------------------------------------------------------
    // 2. Instantiate Device Under Test (DUT)
    // ---------------------------------------------------------
    top dut (
        .CLK(CLK),
        .rst(rst),
        .LCD_PCLK(LCD_PCLK),
        .VSYNC(VSYNC),
        .HSYNC(HSYNC),
        .DE(DE),
        .LCD_CLK(LCD_CLK),
        .LCD_DEN(LCD_DEN),
        .LCD_R(LCD_R),
        .LCD_G(LCD_G),
        .LCD_B(LCD_B),
        .XCLK(XCLK)
    );

    // ---------------------------------------------------------
    // 3. Clock Generation
    // ---------------------------------------------------------
    // CLK (System/LCD Clocks) ~ 25MHz (40ns period)
    always begin
        CLK = 0;
        #20;
        CLK = 1;
        #20;
    end

    // LCD_PCLK (Camera PCLK) ~ 24MHz asynchronous (41.67ns period)
    always begin
        LCD_PCLK = 0;
        #20.83;
        LCD_PCLK = 1;
        #20.83;
    end

    // ---------------------------------------------------------
    // 4. Tasks for Stimulus Generation
    // ---------------------------------------------------------
    
    // Sends one row of camera data (640 pixels = 1280 clock cycles)
    task automatic send_camera_row(input int row_index);
        begin
            @(posedge LCD_PCLK);
            HSYNC = 1; // Assert href
            
            for (int p = 0; p < 640; p++) begin
                // Byte 1: High byte of RGB565 (Red + Upper Green)
                DE = {3'b111_00, 3'b111}; // Dummy pattern
                @(posedge LCD_PCLK);
                
                // Byte 2: Low byte of RGB565 (Lower Green + Blue)
                DE = {3'b000_11, 5'b11111}; 
                @(posedge LCD_PCLK);
            end
            
            // End of row (horizontal blanking gap)
            HSYNC = 0;
            DE = 8'h00;
            repeat (40) @(posedge LCD_PCLK); 
        end
    endtask

    // Sends an entire camera frame (480 rows)
    task automatic send_camera_frame();
        begin
            $display("[TB] Starting Video Frame Generation.");
            // 1. VSYNC Pulse to reset pixel_capture state
            VSYNC = 1;
            repeat (10) @(posedge LCD_PCLK);
            VSYNC = 0;
            repeat (20) @(posedge LCD_PCLK); // Front porch space

            // 2. Loop through all 480 video rows
            for (int r = 0; r < 480; r++) begin
                send_camera_row(r);
            end
            
            $display("[TB] Video Frame Finished.");
        end
    endtask

    // ---------------------------------------------------------
    // 5. Test Vectors
    // ---------------------------------------------------------
    initial begin
        // Initialize Inputs
        rst = 1;
        VSYNC = 0;
        HSYNC = 0;
        DE = 8'h00;

        // Hold reset for a few cycles
        repeat (10) @(posedge CLK);
        $display("[TB] Releasing System Reset.");
        rst = 0;
        
        // Wait for system to settle
        repeat (5) @(posedge LCD_PCLK);

        // Send two complete frames to verify dual-buffer line swapping
        send_camera_frame();
        repeat (100) @(posedge LCD_PCLK); // Inter-frame delay
        send_camera_frame();

        // Let LCD finish drawing the remaining active scanlines if needed
        repeat (2000) @(posedge CLK);

        $display("[TB] Simulation Complete.");
        $finish;
    end

    // ---------------------------------------------------------
    // 6. Monitor Signals in the Console
    // ---------------------------------------------------------
    initial begin
        $monitor("Time=%0t | LCD_DEN=%b | RGB Output={5'h%h, 6'h%h, 5'h%h}", 
                 $time, LCD_DEN, LCD_R, LCD_G, LCD_B);
    end

    // Optional: Dump waveforms for GTKWave/Vivado Simulator
    initial begin
        $dumpfile("dump.vcd");
        $dumpvars(0, tb_top);
    end

endmodule