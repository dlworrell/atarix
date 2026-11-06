`timescale 1ns / 1ps
/*
 * Northbridge bus shim.
 *
 * Captures W65C816 write cycles to the mailbox TX window and delivers each
 * written byte once, in order, to the system-clock (clk) domain.
 *
 * Two clock domains:
 *
 *   PHI2 domain   Data, chip select and R/W are registered on the PHI2 falling
 *                 edge, the instant the W65C816 guarantees them (write data
 *                 and address hold only ~10 ns after the fall: tDHW, tAH).
 *                 Sampling them later from the system clock misses or corrupts
 *                 writes (ATX-RTL-SHIM-001, datasheet-timing co-simulation).
 *
 *   clk domain    A toggle flips once per captured write. It crosses through a
 *                 two-flop synchronizer; a third flop detects the change, and
 *                 the captured byte, which is held stable until the next PHI2
 *                 fall, is then copied out with shim_valid for one clk cycle.
 *
 * Rate requirement (proved in sim/formal/bus_shim_formal.sv): every PHI2
 * period must be longer than the synchronizer latency, i.e. at least
 * 4 system-clock periods; see docs/verification/cpu-card-software-testbench.md
 * for the derived clock table. Falling-edge sampling uses PHI2 as a clock: on
 * the ECP5 route PHI2 to a clock-capable input.
 *
 * Framing (unchanged rule, still an open decision): shim_sof marks the first
 * captured write after a PHI2 cycle in which chip select was inactive.
 * Interface is unchanged from the scaffold.
 */

module atx_northbridge_bus_shim (
    input  wire       clk,
    input  wire       rst_n,

    input  wire [7:0] phi2_d_in,
    input  wire       phi2,
    input  wire       cs_n,
    input  wire       rw,

    output reg  [7:0] shim_data_out = 8'h00,
    output reg        shim_valid = 1'b0,
    output reg        shim_sof = 1'b0
`ifdef FORMAL
    // Internal state exposed only to the formal harness (sim/formal).
    ,
    output wire [7:0] f_cap_data,
    output wire       f_cap_sof,
    output wire       f_cap_toggle,
    output wire       f_prev_cs,
    output wire       f_sync_1,
    output wire       f_sync_2,
    output wire       f_sync_seen
`endif
);

    // Every register has a power-up value (ECP5 FPGA initialization) as well
    // as the asynchronous reset, so the PHI2 domain is defined even if reset
    // is asserted from time zero and PHI2 has not yet toggled.

    // ------------------------------------------------------------------
    // PHI2 domain: capture at the falling edge.
    // ------------------------------------------------------------------
    reg [7:0] cap_data = 8'h00;
    reg       cap_sof = 1'b0;
    reg       cap_toggle = 1'b0;
    reg       prev_cs_active = 1'b0;

    always @(negedge phi2 or negedge rst_n) begin
        if (!rst_n) begin
            cap_data <= 8'h00;
            cap_sof <= 1'b0;
            cap_toggle <= 1'b0;
            prev_cs_active <= 1'b0;
        end else begin
            prev_cs_active <= !cs_n;
            if (!cs_n && !rw) begin
                cap_data <= phi2_d_in;
                cap_sof <= !prev_cs_active;
                cap_toggle <= !cap_toggle;
            end
        end
    end

    // ------------------------------------------------------------------
    // clk domain: synchronize the toggle, then copy the held capture.
    // ------------------------------------------------------------------
    reg sync_1 = 1'b0;
    reg sync_2 = 1'b0;
    reg sync_seen = 1'b0;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            sync_1 <= 1'b0;
            sync_2 <= 1'b0;
            sync_seen <= 1'b0;
            shim_data_out <= 8'h00;
            shim_valid <= 1'b0;
            shim_sof <= 1'b0;
        end else begin
            sync_1 <= cap_toggle;
            sync_2 <= sync_1;
            sync_seen <= sync_2;
            shim_valid <= 1'b0;
            shim_sof <= 1'b0;
            if (sync_2 != sync_seen) begin
`ifdef SHIM_MUTANT_LIVE_DATA
                // Formal negative control: read the live bus instead of the
                // held capture. Must be caught by the proof.
                shim_data_out <= phi2_d_in;
`else
                shim_data_out <= cap_data;
`endif
                shim_sof <= cap_sof;
                shim_valid <= 1'b1;
            end
        end
    end

`ifdef FORMAL
    assign f_cap_data = cap_data;
    assign f_cap_sof = cap_sof;
    assign f_cap_toggle = cap_toggle;
    assign f_prev_cs = prev_cs_active;
    assign f_sync_1 = sync_1;
    assign f_sync_2 = sync_2;
    assign f_sync_seen = sync_seen;
`endif

endmodule
