// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Co-simulation testbench: replays the emulated CPU card's recorded W65C816
// bus cycles into rtl/atarix/atx_northbridge_bus_shim.v and logs every byte
// the shim captures.
//
// Stimulus (one 32-bit hex word per CPU bus cycle, from
// sim/rtl-cosim/run_bus_shim_cosim.py):
//   bit 9    cs_n  (0 when the cycle addresses the mailbox TX frame window)
//   bit 8    rw    (1 read, 0 write)
//   bits 7:0 data
// Each CPU cycle is one PHI2 period of 8 system clocks (PHI2 high for 4).
// The shim samples on the PHI2 falling edge, as the W65C816 latches write data.

`timescale 1ns / 1ps

module bus_shim_cosim_tb;
    parameter MAX_CYCLES = 262144;

    reg        clk = 1'b0;
    reg        rst_n = 1'b0;
    reg  [7:0] phi2_d_in = 8'h00;
    reg        phi2 = 1'b0;
    reg        cs_n = 1'b1;
    reg        rw = 1'b1;
    wire [7:0] shim_data_out;
    wire       shim_valid;
    wire       shim_sof;

    reg [31:0] stimulus [0:MAX_CYCLES-1];
    integer cycle_count;
    integer i;
    integer k;
    integer out;
    integer captured;
    // Paths are packed strings: 8 bits per character, up to 1024 characters.
    reg [8*1024-1:0] stimulus_path;
    reg [8*1024-1:0] output_path;

    atx_northbridge_bus_shim dut (
        .clk(clk),
        .rst_n(rst_n),
        .phi2_d_in(phi2_d_in),
        .phi2(phi2),
        .cs_n(cs_n),
        .rw(rw),
        .shim_data_out(shim_data_out),
        .shim_valid(shim_valid),
        .shim_sof(shim_sof)
    );

    always #5 clk = ~clk;

    always @(posedge clk) begin
        if (shim_valid) begin
            $fwrite(out, "%02x %0d\n", shim_data_out, shim_sof);
            captured = captured + 1;
        end
    end

    initial begin
        if (!$value$plusargs("stimulus=%s", stimulus_path)) begin
            $display("FAIL missing +stimulus=");
            $finish;
        end
        if (!$value$plusargs("output=%s", output_path)) begin
            $display("FAIL missing +output=");
            $finish;
        end
        if (!$value$plusargs("cycles=%d", cycle_count)) begin
            $display("FAIL missing +cycles=");
            $finish;
        end
        if (cycle_count > MAX_CYCLES) begin
            $display("FAIL stimulus has %0d cycles, testbench limit is %0d", cycle_count, MAX_CYCLES);
            $finish;
        end

        $readmemh(stimulus_path, stimulus, 0, cycle_count - 1);
        out = $fopen(output_path, "w");
        captured = 0;

        repeat (4) @(posedge clk);
        rst_n = 1'b1;
        repeat (2) @(posedge clk);

        for (i = 0; i < cycle_count; i = i + 1) begin
            // PHI2 low: address/control phase.
            phi2 = 1'b0;
            cs_n = stimulus[i][9];
            rw = stimulus[i][8];
            phi2_d_in = stimulus[i][7:0];
            for (k = 0; k < 4; k = k + 1) @(posedge clk);
            // PHI2 high: data phase; falling edge at the end latches writes.
            phi2 = 1'b1;
            for (k = 0; k < 4; k = k + 1) @(posedge clk);
        end
        phi2 = 1'b0;
        cs_n = 1'b1;
        rw = 1'b1;
        repeat (8) @(posedge clk);

        $fclose(out);
        $display("COSIM cycles=%0d captured=%0d", cycle_count, captured);
        $finish;
    end
endmodule
