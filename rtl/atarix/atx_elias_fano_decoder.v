`timescale 1ns / 1ps
/*
 * Elias-Fano decoder scaffold.
 *
 * This module provides a bounded 32-bit-word select/decode primitive suitable
 * for simulation. It is not the final optimized rank/select fabric.
 */

module atx_elias_fano_decoder (
    input  wire [31:0] high_bits_word,
    input  wire [31:0] low_bits_word,
    input  wire [3:0]  target_offset,
    input  wire [4:0]  num_low_bits,
    output reg  [31:0] resolved_row_id,
    output reg         decode_valid,
    output reg         decode_error
);

    integer idx;
    integer count;
    reg [5:0] select_index;
    reg found;
    reg [31:0] target_offset32;
    reg [31:0] low_bit_shift;
    reg [31:0] low_mask;
    reg [31:0] low_value;
    reg [31:0] high_value;

    always @(*) begin
        select_index = 6'd0;
        count = 0;
        found = 1'b0;
        target_offset32 = {28'h0, target_offset};
        low_bit_shift = {28'h0, target_offset} * {27'h0, num_low_bits};
        decode_valid = 1'b0;
        decode_error = 1'b0;
        resolved_row_id = 32'h0;
        low_mask = 32'h0;
        low_value = 32'h0;
        high_value = 32'h0;

        for (idx = 0; idx < 32; idx = idx + 1) begin
            if (high_bits_word[idx] && !found) begin
                if (count == target_offset32) begin
                    select_index = idx[5:0];
                    found = 1'b1;
                end
                count = count + 1;
            end
        end

        if (!found) begin
            decode_error = 1'b1;
        end else if (num_low_bits > 5'd16) begin
            decode_error = 1'b1;
        end else begin
            low_mask = (32'h1 << num_low_bits) - 32'h1;
            low_value = (low_bits_word >> low_bit_shift) & low_mask;
            high_value = {26'h0, select_index} - target_offset32;
            resolved_row_id = (high_value << num_low_bits) | low_value;
            decode_valid = 1'b1;
        end
    end

endmodule
