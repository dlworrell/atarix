// SPDX-FileCopyrightText: Donovan Worrell
// SPDX-License-Identifier: Apache-2.0
//
// Formal harness for rtl/atarix/atx_northbridge_bus_shim.v.
//
// Time is the global formal clock. clk toggles every step (period 2 steps);
// PHI2 is an independent input whose every phase lasts at least
// PHI2_MIN_HALF steps. cs_n, rw and the data bus are unconstrained: the
// solver may change them at any step, including at PHI2 edges.
//
// A reference model records each write the W65C816 performs (chip select and
// write active at a PHI2 falling edge). The proof requires that:
//   1. nothing is delivered that was not written (no duplicates);
//   2. no write is lost (at most one write outstanding at any time);
//   3. every delivered byte and SOF flag equal the outstanding write's;
//   4. an outstanding write is delivered within MAX_LATENCY steps.
// scripts/rtl/run_shim_formal.sh runs bounded model checking and k-induction,
// sweeps PHI2_MIN_HALF to find the minimum clock ratio, and requires mutants
// to fail.

`default_nettype none

module bus_shim_formal (
    input wire       clk,
    input wire       phi2,
    input wire       cs_n,
    input wire       rw,
    input wire [7:0] d
);
`ifndef PHI2_MIN_HALF
    `define PHI2_MIN_HALF 4
`endif
`ifndef MAX_LATENCY
    `define MAX_LATENCY 6
`endif
    localparam integer MIN_HALF = `PHI2_MIN_HALF;
    localparam integer MAX_LATENCY = `MAX_LATENCY;

    wire [7:0] q;
    wire       valid;
    wire       sof;
    wire [7:0] f_cap_data;
    wire       f_cap_sof, f_cap_toggle, f_prev_cs, f_sync_1, f_sync_2, f_sync_seen;

    atx_northbridge_bus_shim dut (
        .clk(clk),
        .rst_n(1'b1),          // power-up values define the initial state
        .phi2_d_in(d),
        .phi2(phi2),
        .cs_n(cs_n),
        .rw(rw),
        .shim_data_out(q),
        .shim_valid(valid),
        .shim_sof(sof)
`ifndef LEGACY_SHIM_CHECK
        ,
        .f_cap_data(f_cap_data),
        .f_cap_sof(f_cap_sof),
        .f_cap_toggle(f_cap_toggle),
        .f_prev_cs(f_prev_cs),
        .f_sync_1(f_sync_1),
        .f_sync_2(f_sync_2),
        .f_sync_seen(f_sync_seen)
`endif
    );

    // ------------------------------------------------------------------
    // Previous-step copies of the inputs (global clock).
    // ------------------------------------------------------------------
    reg       started = 1'b0;
    reg       prev_clk = 1'b0;
    reg       prev_phi2 = 1'b0;
    reg       prev_cs_n = 1'b1;
    reg       prev_rw = 1'b1;
    reg [7:0] prev_d = 8'd0;
    reg [7:0] phase_len = 8'd255;   // steps the current PHI2 phase has lasted (saturating)

    always @($global_clock) begin
        started <= 1'b1;
        prev_clk <= clk;
        prev_phi2 <= phi2;
        prev_cs_n <= cs_n;
        prev_rw <= rw;
        prev_d <= d;
        if (phi2 != prev_phi2) begin
            phase_len <= 8'd0;
        end else if (phase_len != 8'd255) begin
            phase_len <= phase_len + 8'd1;
        end
    end

    wire phi2_fell = started && prev_phi2 && !phi2;
    wire clk_rose = started && !prev_clk && clk;

    // Clock assumptions.
    always @(*) begin
        if (started) begin
            assume (clk != prev_clk);
            if (phi2 != prev_phi2) assume (phase_len >= MIN_HALF - 1);
        end
    end

    // ------------------------------------------------------------------
    // Reference model: what the CPU wrote, recorded at each PHI2 fall from
    // the values present on the bus just before that edge.
    //
    // After clk2fflogic, a register clocked by PHI2 or clk takes its new
    // value in the same global step as the edge. The reference therefore
    // works on "now" values (the registered value updated by this step's
    // event) so both sides are compared in the same step.
    // ------------------------------------------------------------------
    reg [7:0] written_q = 8'd0;
    reg [7:0] delivered_q = 8'd0;
    reg [7:0] ref_data_q = 8'd0;
    reg       ref_sof_q = 1'b0;
    reg       ref_prev_cs_q = 1'b0;
    reg [7:0] pending_age = 8'd0;

    wire cpu_write = phi2_fell && !prev_cs_n && !prev_rw;
    // A delivery is a shim_valid pulse, seen at the clk edge that raises it.
    wire deliver = clk_rose && valid;

    wire [7:0] written = written_q + (cpu_write ? 8'd1 : 8'd0);
    wire [7:0] delivered = delivered_q + (deliver ? 8'd1 : 8'd0);
    wire [7:0] ref_data = cpu_write ? prev_d : ref_data_q;
    wire       ref_sof = cpu_write ? !ref_prev_cs_q : ref_sof_q;
    wire       ref_prev_cs = phi2_fell ? !prev_cs_n : ref_prev_cs_q;
    wire [7:0] outstanding = written - delivered;

    always @($global_clock) begin
        written_q <= written;
        delivered_q <= delivered;
        ref_data_q <= ref_data;
        ref_sof_q <= ref_sof;
        ref_prev_cs_q <= ref_prev_cs;
        if (outstanding != 8'd0) begin
            if (pending_age != 8'd255) pending_age <= pending_age + 8'd1;
        end else begin
            pending_age <= 8'd0;
        end
    end

    always @(*) begin
        // 1 and 2: never more delivered than written; never two outstanding.
        assert (outstanding <= 8'd1);
        // 3: the delivered byte is the outstanding write's byte and frame flag.
        if (deliver) begin
            assert (outstanding == 8'd0);       // this pulse delivers the one outstanding write
            assert (q == ref_data);
            assert (sof == ref_sof);
        end
        // 4: bounded latency.
        assert (pending_age <= MAX_LATENCY);

`ifndef LEGACY_SHIM_CHECK
        // Invariants tying the shim's state to the reference model. They are
        // checked like any other assertion and let k-induction close.
        // (LEGACY_SHIM_CHECK: properties 1-4 only, for designs without the
        // formal debug ports, such as the original scaffold.)
        assert (f_cap_toggle == written[0]);
        assert (f_sync_seen == delivered[0]);
        assert (f_prev_cs == ref_prev_cs);
        if (outstanding == 8'd0 && !deliver) begin
            assert (f_sync_1 == f_cap_toggle);
            assert (f_sync_2 == f_cap_toggle);
        end
        assert (f_cap_data == ref_data);
        assert (f_cap_sof == ref_sof);
`endif
    end
endmodule

`default_nettype wire
