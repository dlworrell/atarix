`timescale 1ns / 1ps
/*
 * 16-lane control-byte probe core.
 *
 * This module checks one 128-bit control-byte group. It is deliberately
 * stateless and carries no authority. Authorization must happen before this
 * module is enabled by a caller.
 */

module atx_simd_probe_core (
    input  wire [127:0] control_bytes,
    input  wire [6:0]   target_h2,
    output wire         match_found,
    output reg  [3:0]   match_offset,
    output wire [15:0]  lane_matches
);

    genvar i;
    generate
        for (i = 0; i < 16; i = i + 1) begin : g_lane_compare
            assign lane_matches[i] =
                (control_bytes[(i * 8) + 7] == 1'b1) &&
                (control_bytes[(i * 8) +: 7] == target_h2);
        end
    endgenerate

    assign match_found = |lane_matches;

    always @(*) begin
        casex (lane_matches)
            16'bxxxx_xxxx_xxxx_xxx1: match_offset = 4'd0;
            16'bxxxx_xxxx_xxxx_xx10: match_offset = 4'd1;
            16'bxxxx_xxxx_xxxx_x100: match_offset = 4'd2;
            16'bxxxx_xxxx_xxxx_1000: match_offset = 4'd3;
            16'bxxxx_xxxx_xxx1_0000: match_offset = 4'd4;
            16'bxxxx_xxxx_xx10_0000: match_offset = 4'd5;
            16'bxxxx_xxxx_x100_0000: match_offset = 4'd6;
            16'bxxxx_xxxx_1000_0000: match_offset = 4'd7;
            16'bxxxx_xxx1_0000_0000: match_offset = 4'd8;
            16'bxxxx_xx10_0000_0000: match_offset = 4'd9;
            16'bxxxx_x100_0000_0000: match_offset = 4'd10;
            16'bxxxx_1000_0000_0000: match_offset = 4'd11;
            16'bxxx1_0000_0000_0000: match_offset = 4'd12;
            16'bxx10_0000_0000_0000: match_offset = 4'd13;
            16'bx100_0000_0000_0000: match_offset = 4'd14;
            16'b1000_0000_0000_0000: match_offset = 4'd15;
            default:                 match_offset = 4'd0;
        endcase
    end

endmodule
