`timescale 1ns / 1ps

module atx_scalar_probe_core (
    input  wire        clk,
    input  wire        rst_n,
    input  wire        start,
    input  wire [127:0] control_bytes,
    input  wire [6:0]   target_h2,
    output reg         done,
    output reg         match_found,
    output reg  [3:0]  match_offset,
    output reg  [4:0]  cycles_used
);

    reg [4:0] lane;
    reg [7:0] lane_byte;

    always @(*) begin
        lane_byte = control_bytes[(lane[3:0] * 8) +: 8];
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            done <= 1'b1;
            match_found <= 1'b0;
            match_offset <= 4'h0;
            cycles_used <= 5'h0;
            lane <= 5'h0;
        end else if (start) begin
            done <= 1'b0;
            match_found <= 1'b0;
            match_offset <= 4'h0;
            cycles_used <= 5'h0;
            lane <= 5'h0;
        end else if (!done) begin
            cycles_used <= cycles_used + 1'b1;
            if ((lane_byte[7] == 1'b1) && (lane_byte[6:0] == target_h2)) begin
                match_found <= 1'b1;
                match_offset <= lane[3:0];
                done <= 1'b1;
            end else if (lane == 5'd15) begin
                done <= 1'b1;
            end else begin
                lane <= lane + 1'b1;
            end
        end
    end

endmodule
