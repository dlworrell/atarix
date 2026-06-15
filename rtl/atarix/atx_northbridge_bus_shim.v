`timescale 1ns / 1ps
/*
 * Northbridge bus shim scaffold.
 *
 * Samples W65C816-style write cycles on PHI2 falling edge and emits a simple
 * synchronous byte stream for future mailbox frame assembly.
 */

module atx_northbridge_bus_shim (
    input  wire       clk,
    input  wire       rst_n,

    input  wire [7:0] phi2_d_in,
    input  wire       phi2,
    input  wire       cs_n,
    input  wire       rw,

    output reg  [7:0] shim_data_out,
    output reg        shim_valid,
    output reg        shim_sof
);

    reg old_phi2;
    reg processing_frame;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            old_phi2 <= 1'b0;
            shim_data_out <= 8'h00;
            shim_valid <= 1'b0;
            shim_sof <= 1'b0;
            processing_frame <= 1'b0;
        end else begin
            old_phi2 <= phi2;
            shim_valid <= 1'b0;
            shim_sof <= 1'b0;

            if (old_phi2 && !phi2 && !cs_n && !rw) begin
                shim_data_out <= phi2_d_in;
                shim_valid <= 1'b1;
                if (!processing_frame) begin
                    shim_sof <= 1'b1;
                    processing_frame <= 1'b1;
                end
            end else if (cs_n) begin
                processing_frame <= 1'b0;
            end
        end
    end

endmodule
