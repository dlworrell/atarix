`timescale 1ns / 1ps
/*
 * ATX-SPEC-020 system wrapper scaffold.
 *
 * Binds the lookup accelerator to an audit log window. This is a structural
 * integration point for future bus shim and memory fabric work.
 */

module atx_spec_020_system_wrapper (
    input  wire        clk,
    input  wire        rst_n,

    input  wire        req_valid,
    output wire        req_ready,
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

    output wire         resp_valid,
    input  wire         resp_ready,
    output wire [7:0]   resp_status,
    output wire [31:0]  resp_resolved_id,
    output wire [31:0]  resp_resolved_generation,
    output wire [3:0]   resp_match_offset,
    output wire [31:0]  resp_cycles,

    input  wire [7:0]   audit_read_addr,
    output wire [7:0]   audit_read_data
);

    wire        audit_valid;
    wire [31:0] audit_sequence;
    wire [7:0]  audit_status;
    wire [31:0] audit_registry_id;
    wire [31:0] audit_generation;
    wire [7:0]  audit_probe_count;
    wire [3:0]  audit_match_offset;

    wire        probe_valid;
    wire [31:0] probe_addr;
    wire [31:0] probe_next_addr;
    wire [7:0]  audit_window_head;
    wire        audit_window_wrapped;

    wire [7:0] audit_projection_byte;
    wire       audit_byte_valid;

    assign audit_byte_valid = audit_valid;
    assign audit_projection_byte =
        audit_status ^
        {7'h0, ^audit_sequence} ^
        {7'h0, ^audit_registry_id} ^
        {7'h0, ^audit_generation} ^
        audit_probe_count ^
        {4'h0, audit_match_offset} ^
        {7'h0, ^probe_addr} ^
        {7'h0, ^probe_next_addr} ^
        {7'h0, probe_valid} ^
        audit_window_head ^
        {7'h0, audit_window_wrapped};

    atx_spec_020_accelerator accelerator (
        .clk(clk),
        .rst_n(rst_n),
        .req_valid(req_valid),
        .req_ready(req_ready),
        .req_sequence(req_sequence),
        .req_type(req_type),
        .req_source_ring(req_source_ring),
        .req_target_ring(req_target_ring),
        .req_crc_ok(req_crc_ok),
        .req_ring_ok(req_ring_ok),
        .req_capability_ok(req_capability_ok),
        .req_policy_ok(req_policy_ok),
        .req_registry_id(req_registry_id),
        .req_key_hash(req_key_hash),
        .req_expected_generation(req_expected_generation),
        .req_query_type(req_query_type),
        .req_probe_limit(req_probe_limit),
        .req_projection_flags(req_projection_flags),
        .table_control_bytes(table_control_bytes),
        .table_generation(table_generation),
        .table_base_resolved_id(table_base_resolved_id),
        .table_lookup_valid(table_lookup_valid),
        .probe_valid(probe_valid),
        .probe_addr(probe_addr),
        .probe_next_addr(probe_next_addr),
        .resp_valid(resp_valid),
        .resp_ready(resp_ready),
        .resp_status(resp_status),
        .resp_resolved_id(resp_resolved_id),
        .resp_resolved_generation(resp_resolved_generation),
        .resp_match_offset(resp_match_offset),
        .resp_cycles(resp_cycles),
        .audit_valid(audit_valid),
        .audit_ready(1'b1),
        .audit_sequence(audit_sequence),
        .audit_status(audit_status),
        .audit_registry_id(audit_registry_id),
        .audit_generation(audit_generation),
        .audit_probe_count(audit_probe_count),
        .audit_match_offset(audit_match_offset)
    );

    atx_audit_log_window audit_window (
        .clk(clk),
        .rst_n(rst_n),
        .write_data(audit_projection_byte),
        .write_en(audit_byte_valid),
        .read_addr(audit_read_addr),
        .read_data(audit_read_data),
        .head_pointer(audit_window_head),
        .wrapped(audit_window_wrapped)
    );

endmodule
