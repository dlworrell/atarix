`timescale 1ns / 1ps
/*
 * ATX-SPEC-020 lookup accelerator.
 */

module atx_spec_020_accelerator (
    input  wire        clk,
    input  wire        rst_n,

    input  wire        req_valid,
    output reg         req_ready,

    input  wire [31:0] req_sequence,
    input  wire [15:0] req_type,
    input  wire [15:0] req_source_ring,
    input  wire [15:0] req_target_ring,
    input  wire        req_crc_ok,
    input  wire        req_ring_ok,
    input  wire        req_capability_ok,
    input  wire        req_policy_ok,

    input  wire [31:0] req_registry_id,
    input  wire [31:0] req_key_hash,
    input  wire [31:0] req_expected_generation,
    input  wire [7:0]  req_query_type,
    input  wire [7:0]  req_probe_limit,
    input  wire [7:0]  req_projection_flags,

    input  wire [127:0] table_control_bytes,
    input  wire [31:0]  table_generation,
    input  wire [31:0]  table_base_resolved_id,
    input  wire         table_lookup_valid,

    output reg          probe_valid,
    output reg  [31:0]  probe_addr,
    output wire [31:0]  probe_next_addr,

    output reg         resp_valid,
    input  wire        resp_ready,
    output reg [7:0]   resp_status,
    output reg [31:0]  resp_resolved_id,
    output reg [31:0]  resp_resolved_generation,
    output reg [3:0]   resp_match_offset,
    output reg [31:0]  resp_cycles,

    output reg         audit_valid,
    input  wire        audit_ready,
    output reg [31:0]  audit_sequence,
    output reg [7:0]   audit_status,
    output reg [31:0]  audit_registry_id,
    output reg [31:0]  audit_generation,
    output reg [7:0]   audit_probe_count,
    output reg [3:0]   audit_match_offset
);

    localparam TYPE_INDEX_QUERY = 16'h0020;

    localparam STATUS_OK             = 8'h00;
    localparam STATUS_INVALID_HEADER = 8'h02;
    localparam STATUS_CRC_MISMATCH   = 8'h04;
    localparam STATUS_SEQUENCE_ERROR = 8'h05;
    localparam STATUS_RING_DENIED    = 8'h09;
    localparam STATUS_POLICY_DENIED  = 8'h0A;
    localparam STATUS_CAP_DENIED     = 8'h0B;
    localparam STATUS_TIMEOUT        = 8'h0E;

    localparam STATE_IDLE        = 4'd0;
    localparam STATE_VALIDATE    = 4'd1;
    localparam STATE_AUTH        = 4'd2;
    localparam STATE_FETCH       = 4'd3;
    localparam STATE_PROBE       = 4'd4;
    localparam STATE_STEP        = 4'd5;
    localparam STATE_AUDIT       = 4'd6;
    localparam STATE_RESPOND     = 4'd7;

    reg [3:0] state;
    reg [31:0] cycle_counter;
    reg [7:0] probe_count;
    reg [6:0] h2_fingerprint;
    reg [31:0] table_mask;
    reg response_seen;

    wire simd_hit;
    wire [3:0] simd_match_offset;
    wire [15:0] simd_lane_matches;
    wire [31:0] krapivin_offset;

    /* Reserved interface fields for future policy/audit expansion. */
    /* verilator lint_off UNUSEDSIGNAL */
    wire [47:0] reserved_request_fields;
    wire [15:0] reserved_lane_observer;
    assign reserved_request_fields = {
        req_source_ring,
        req_target_ring,
        req_query_type,
        req_projection_flags
    };
    assign reserved_lane_observer = simd_lane_matches;
    /* verilator lint_on UNUSEDSIGNAL */

    atx_simd_probe_core simd_probe_core (
        .control_bytes(table_control_bytes),
        .target_h2(h2_fingerprint),
        .match_found(simd_hit),
        .match_offset(simd_match_offset),
        .lane_matches(simd_lane_matches)
    );

    atx_krapivin_stepper krapivin_stepper (
        .current_base(probe_addr),
        .current_attempt(probe_count),
        .table_mask(table_mask),
        .next_base(probe_next_addr),
        .non_linear_offset(krapivin_offset)
    );

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= STATE_IDLE;
            req_ready <= 1'b1;
            probe_valid <= 1'b0;
            probe_addr <= 32'h00000000;
            resp_valid <= 1'b0;
            resp_status <= STATUS_OK;
            resp_resolved_id <= 32'h00000000;
            resp_resolved_generation <= 32'h00000000;
            resp_match_offset <= 4'h0;
            resp_cycles <= 32'h00000000;
            audit_valid <= 1'b0;
            audit_sequence <= 32'h00000000;
            audit_status <= STATUS_OK;
            audit_registry_id <= 32'h00000000;
            audit_generation <= 32'h00000000;
            audit_probe_count <= 8'h00;
            audit_match_offset <= 4'h0;
            cycle_counter <= 32'h00000000;
            probe_count <= 8'h00;
            h2_fingerprint <= 7'h00;
            table_mask <= 32'hFFFFFFFF;
            response_seen <= 1'b0;
        end else begin
            cycle_counter <= cycle_counter + 1'b1;

            case (state)
                STATE_IDLE: begin
                    req_ready <= 1'b1;
                    probe_valid <= 1'b0;
                    resp_valid <= 1'b0;
                    audit_valid <= 1'b0;
                    probe_count <= 8'h00;
                    response_seen <= 1'b0;
                    if (req_valid) begin
                        req_ready <= 1'b0;
                        h2_fingerprint <= req_key_hash[6:0];
                        probe_addr <= table_base_resolved_id;
                        table_mask <= 32'hFFFFFFFF;
                        resp_resolved_id <= 32'hFFFFFFFF;
                        resp_resolved_generation <= 32'h00000000;
                        resp_match_offset <= 4'h0;
                        resp_status <= STATUS_OK;
                        state <= STATE_VALIDATE;
`ifndef SYNTHESIS
                        $display("ATX020 START seq=%0d type=0x%04x registry=0x%08x key=0x%08x gen=%0d limit=%0d",
                                 req_sequence, req_type, req_registry_id, req_key_hash,
                                 req_expected_generation, req_probe_limit);
`endif
                    end
                end

                STATE_VALIDATE: begin
                    if (req_type != TYPE_INDEX_QUERY) begin
                        resp_status <= STATUS_INVALID_HEADER;
                        state <= STATE_AUDIT;
                    end else if (!req_crc_ok) begin
                        resp_status <= STATUS_CRC_MISMATCH;
                        state <= STATE_AUDIT;
                    end else begin
                        state <= STATE_AUTH;
                    end
                end

                STATE_AUTH: begin
                    if (!req_ring_ok) begin
                        resp_status <= STATUS_RING_DENIED;
                        state <= STATE_AUDIT;
                    end else if (!req_capability_ok) begin
                        resp_status <= STATUS_CAP_DENIED;
                        state <= STATE_AUDIT;
                    end else if (!req_policy_ok) begin
                        resp_status <= STATUS_POLICY_DENIED;
                        state <= STATE_AUDIT;
                    end else begin
                        probe_valid <= 1'b1;
                        state <= STATE_FETCH;
                    end
                end

                STATE_FETCH: begin
                    probe_valid <= 1'b0;
                    if (table_lookup_valid) begin
                        state <= STATE_PROBE;
                    end else begin
                        resp_status <= STATUS_TIMEOUT;
                        state <= STATE_AUDIT;
                    end
                end

                STATE_PROBE: begin
                    probe_count <= probe_count + 1'b1;
                    if (simd_hit) begin
                        resp_status <= STATUS_OK;
                        resp_match_offset <= simd_match_offset;
                        resp_resolved_id <= probe_addr + {28'h0, simd_match_offset};
                        resp_resolved_generation <= table_generation;
                        state <= STATE_AUDIT;
                    end else if (probe_count >= req_probe_limit) begin
                        resp_status <= STATUS_SEQUENCE_ERROR;
                        state <= STATE_AUDIT;
                    end else begin
                        state <= STATE_STEP;
                    end
                end

                STATE_STEP: begin
                    probe_addr <= probe_next_addr;
                    probe_valid <= 1'b1;
`ifndef SYNTHESIS
                    $display("ATX020 STEP seq=%0d attempt=%0d next_addr=0x%08x offset=%0d",
                             req_sequence, probe_count, probe_next_addr, krapivin_offset);
`endif
                    state <= STATE_FETCH;
                end

                STATE_AUDIT: begin
                    probe_valid <= 1'b0;
                    audit_valid <= 1'b1;
                    audit_sequence <= req_sequence;
                    audit_status <= resp_status;
                    audit_registry_id <= req_registry_id;
                    audit_generation <= table_generation;
                    audit_probe_count <= probe_count;
                    audit_match_offset <= resp_match_offset;
                    if (audit_ready) begin
`ifndef SYNTHESIS
                        $display("ATX020 AUDIT seq=%0d status=0x%02x registry=0x%08x gen=%0d probes=%0d offset=%0d",
                                 req_sequence, resp_status, req_registry_id, table_generation,
                                 probe_count, resp_match_offset);
`endif
                        audit_valid <= 1'b0;
                        state <= STATE_RESPOND;
                    end
                end

                STATE_RESPOND: begin
                    resp_valid <= 1'b1;
                    resp_cycles <= cycle_counter;
                    if (!response_seen) begin
                        response_seen <= 1'b1;
`ifndef SYNTHESIS
                        $display("ATX020 RESP seq=%0d status=0x%02x resolved=0x%08x gen=%0d cycles=%0d",
                                 req_sequence, resp_status, resp_resolved_id,
                                 resp_resolved_generation, cycle_counter);
`endif
                    end else if (resp_ready) begin
                        resp_valid <= 1'b0;
                        response_seen <= 1'b0;
                        state <= STATE_IDLE;
                    end
                end

                default: begin
                    resp_status <= STATUS_INVALID_HEADER;
                    state <= STATE_AUDIT;
                end
            endcase
        end
    end

endmodule
