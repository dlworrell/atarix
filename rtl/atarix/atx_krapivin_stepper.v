`timescale 1ns / 1ps
/*
 * Krapivin-style open-addressing probe stepper scaffold.
 *
 * Computes a bounded non-linear probe offset. This module is a sizing/probe
 * aid only and does not grant authority.
 */

module atx_krapivin_stepper (
    input  wire [31:0] current_base,
    input  wire [7:0]  current_attempt,
    input  wire [31:0] table_mask,
    output wire [31:0] next_base,
    output wire [31:0] non_linear_offset
);

    wire [31:0] attempt32;

    assign attempt32 = {24'h0, current_attempt};

    /* Triangular-number step: (i*i + i) / 2. */
    assign non_linear_offset = ((attempt32 * attempt32) + attempt32) >> 1;

    assign next_base = (current_base + non_linear_offset) & table_mask;

endmodule
