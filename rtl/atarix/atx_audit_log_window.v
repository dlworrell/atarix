`timescale 1ns / 1ps

module atx_audit_log_window #(
    parameter BUFFER_DEPTH = 256,
    parameter ADDR_WIDTH = 8
)(
    input  wire                   clk,
    input  wire                   rst_n,
    input  wire [7:0]             write_data,
    input  wire                   write_en,
    input  wire [ADDR_WIDTH-1:0]  read_addr,
    output wire [7:0]             read_data,
    output reg  [ADDR_WIDTH-1:0]  head_pointer,
    output reg                    wrapped
);

    localparam integer LAST_ADDR_INT = BUFFER_DEPTH - 1;

    reg [7:0] storage_matrix [0:BUFFER_DEPTH-1];

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            head_pointer <= {ADDR_WIDTH{1'b0}};
            wrapped <= 1'b0;
        end else if (write_en) begin
            storage_matrix[head_pointer] <= write_data;
            if (head_pointer == LAST_ADDR_INT[ADDR_WIDTH-1:0]) begin
                head_pointer <= {ADDR_WIDTH{1'b0}};
                wrapped <= 1'b1;
            end else begin
                head_pointer <= head_pointer + 1'b1;
            end
        end
    end

    assign read_data = storage_matrix[read_addr];

endmodule
