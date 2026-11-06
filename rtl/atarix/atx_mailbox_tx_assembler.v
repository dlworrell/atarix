`timescale 1ns / 1ps
/*
 * Mailbox TX frame assembler (Fabric Northbridge, system-clock domain).
 *
 * Framing rule (simulator register profile, docs/verification/
 * cpu-card-software-testbench.md): the TX window $00D280-$00D2BF is memory
 * the CPU may write in any order; a frame exists only when the CPU writes
 * $01 (SUBMIT) to CONTROL at $00D201. This is the rule the C Northbridge model
 * uses, so the RTL and C frames can be compared submit by submit.
 *
 * Input: the (address, data) writes delivered by atx_northbridge_bus_shim.v,
 * address = offset within the $00D2xx block.
 *
 * On SUBMIT the 64-byte window is copied into a shadow buffer in one cycle,
 * so the CPU may start writing the next frame at once. The frame is then
 * streamed one byte per clock: the 16-byte v1 header followed by the payload
 * length the header claims, capped at the 48 bytes the window can hold (a
 * longer claim is still streamed at the cap; validation downstream rejects it,
 * as libatarix does in the C model). frame_overrun latches if a SUBMIT arrives
 * while a frame is still streaming; at f_clk >= 4 x f_PHI2 a new SUBMIT needs
 * at least 17 bus writes (68 clocks), longer than the 64-clock stream.
 */

module atx_mailbox_tx_assembler (
    input  wire       clk,
    input  wire       rst_n,

    input  wire       wr_valid,
    input  wire [7:0] wr_addr,
    input  wire [7:0] wr_data,

    output reg        frame_valid = 1'b0,
    output reg  [7:0] frame_data = 8'h00,
    output reg        frame_first = 1'b0,
    output reg        frame_last = 1'b0,
    output reg        frame_overrun = 1'b0
);

    localparam [7:0] REG_CONTROL = 8'h01;
    localparam [7:0] CTL_SUBMIT = 8'h01;
    localparam [7:0] TX_BASE = 8'h80;
    localparam integer WINDOW = 64;
    localparam integer HEADER = 16;

    reg [7:0] tx [0:WINDOW-1];
    reg [7:0] shadow [0:WINDOW-1];
    reg [6:0] stream_len = 7'd0;     // bytes in the frame being streamed
    reg [6:0] stream_pos = 7'd0;
    reg       streaming = 1'b0;

    integer i;
    initial begin
        for (i = 0; i < WINDOW; i = i + 1) begin
            tx[i] = 8'h00;
            shadow[i] = 8'h00;
        end
    end

    wire in_window = wr_addr >= TX_BASE && wr_addr < TX_BASE + WINDOW;
    wire submit = wr_valid && wr_addr == REG_CONTROL && wr_data == CTL_SUBMIT;

    // Claimed payload length (little-endian header bytes 6-7), capped.
    wire [15:0] claimed = {tx[7], tx[6]};
    wire [6:0]  payload = (claimed > 16'd48) ? 7'd48 : claimed[6:0];

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            streaming <= 1'b0;
            stream_len <= 7'd0;
            stream_pos <= 7'd0;
            frame_valid <= 1'b0;
            frame_first <= 1'b0;
            frame_last <= 1'b0;
            frame_overrun <= 1'b0;
        end else begin
            frame_valid <= 1'b0;
            frame_first <= 1'b0;
            frame_last <= 1'b0;

            if (wr_valid && in_window) begin
                tx[wr_addr - TX_BASE] <= wr_data;
            end

            if (submit) begin
                if (streaming) frame_overrun <= 1'b1;
                for (i = 0; i < WINDOW; i = i + 1) shadow[i] <= tx[i];
                stream_len <= 7'd16 + payload;
                stream_pos <= 7'd0;
                streaming <= 1'b1;
            end else if (streaming) begin
                frame_valid <= 1'b1;
                frame_data <= shadow[stream_pos[5:0]];
                frame_first <= stream_pos == 7'd0;
                frame_last <= stream_pos == stream_len - 7'd1;
                if (stream_pos == stream_len - 7'd1) begin
                    streaming <= 1'b0;
                end
                stream_pos <= stream_pos + 7'd1;
            end
        end
    end

endmodule
