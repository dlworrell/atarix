`timescale 1ns / 1ps

module atx_spec_020_accelerator_tb;

    reg clk;
    reg rst_n;

    reg req_valid;
    wire req_ready;

    reg [31:0] req_sequence;
    reg [15:0] req_type;
    reg [15:0] req_source_ring;
    reg [15:0] req_target_ring;
    reg req_crc_ok;
    reg req_ring_ok;
    reg req_capability_ok;
    reg req_policy_ok;

    reg [31:0] req_registry_id;
    reg [31:0] req_key_hash;
    reg [31:0] req_expected_generation;
    reg [7:0] req_query_type;
    reg [7:0] req_probe_limit;
    reg [7:0] req_projection_flags;

    reg [127:0] table_control_bytes;
    reg [31:0] table_generation;
    reg [31:0] table_base_resolved_id;
    reg table_lookup_valid;

    wire resp_valid;
    reg resp_ready;
    wire [7:0] resp_status;
    wire [31:0] resp_resolved_id;
    wire [31:0] resp_resolved_generation;
    wire [3:0] resp_match_offset;
    wire [31:0] resp_cycles;

    wire audit_valid;
    reg audit_ready;
    wire [31:0] audit_sequence;
    wire [7:0] audit_status;
    wire [31:0] audit_registry_id;
    wire [31:0] audit_generation;
    wire [7:0] audit_probe_count;
    wire [3:0] audit_match_offset;

    integer tests_run;
    integer tests_failed;
    integer start_cycle;
    integer end_cycle;
    integer last_cycles;

    localparam TYPE_INDEX_QUERY = 16'h0020;
    localparam STATUS_OK             = 8'h00;
    localparam STATUS_INVALID_HEADER = 8'h02;
    localparam STATUS_CRC_MISMATCH   = 8'h04;
    localparam STATUS_SEQUENCE_ERROR = 8'h05;
    localparam STATUS_RING_DENIED    = 8'h09;
    localparam STATUS_POLICY_DENIED  = 8'h0A;
    localparam STATUS_CAP_DENIED     = 8'h0B;
    localparam STATUS_TIMEOUT        = 8'h0E;

    atx_spec_020_accelerator dut (
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
        .resp_valid(resp_valid),
        .resp_ready(resp_ready),
        .resp_status(resp_status),
        .resp_resolved_id(resp_resolved_id),
        .resp_resolved_generation(resp_resolved_generation),
        .resp_match_offset(resp_match_offset),
        .resp_cycles(resp_cycles),
        .audit_valid(audit_valid),
        .audit_ready(audit_ready),
        .audit_sequence(audit_sequence),
        .audit_status(audit_status),
        .audit_registry_id(audit_registry_id),
        .audit_generation(audit_generation),
        .audit_probe_count(audit_probe_count),
        .audit_match_offset(audit_match_offset)
    );

    always #5 clk = ~clk;

    initial begin
        $dumpfile("atx_spec_020_accelerator_tb.vcd");
        $dumpvars(0, atx_spec_020_accelerator_tb);
    end

    task reset_inputs;
        begin
            req_valid = 1'b0;
            req_sequence = 32'h0;
            req_type = TYPE_INDEX_QUERY;
            req_source_ring = 16'h0001;
            req_target_ring = 16'h0002;
            req_crc_ok = 1'b1;
            req_ring_ok = 1'b1;
            req_capability_ok = 1'b1;
            req_policy_ok = 1'b1;
            req_registry_id = 32'h00001000;
            req_key_hash = 32'h0000002A;
            req_expected_generation = 32'h00000005;
            req_query_type = 8'h00;
            req_probe_limit = 8'h04;
            req_projection_flags = 8'h03;
            table_control_bytes = 128'h0;
            table_generation = 32'h00000005;
            table_base_resolved_id = 32'h00002000;
            table_lookup_valid = 1'b1;
            resp_ready = 1'b1;
            audit_ready = 1'b1;
        end
    endtask

    task set_lane;
        input integer lane;
        input [6:0] fingerprint;
        begin
            table_control_bytes[(lane * 8) +: 8] = {1'b1, fingerprint};
        end
    endtask

    task issue_request;
        begin
            @(posedge clk);
            start_cycle = $time / 10;
            req_valid = 1'b1;
            @(posedge clk);
            req_valid = 1'b0;
            wait(resp_valid == 1'b1);
            end_cycle = $time / 10;
            last_cycles = end_cycle - start_cycle;
            @(posedge clk);
        end
    endtask

    task expect_status;
        input [8*64-1:0] name;
        input [7:0] expected_status;
        begin
            tests_run = tests_run + 1;
            if (resp_status !== expected_status) begin
                tests_failed = tests_failed + 1;
                $display("FAIL %-32s expected status=0x%02x got=0x%02x", name, expected_status, resp_status);
            end else begin
                $display("PASS %-32s status=0x%02x cycles=%0d audit_seq=%0d audit_status=0x%02x probes=%0d offset=%0d resolved=0x%08x",
                         name, resp_status, last_cycles, audit_sequence, audit_status,
                         audit_probe_count, audit_match_offset, resp_resolved_id);
            end
        end
    endtask

    initial begin
        clk = 1'b0;
        rst_n = 1'b0;
        tests_run = 0;
        tests_failed = 0;
        reset_inputs();

        repeat (4) @(posedge clk);
        rst_n = 1'b1;
        repeat (2) @(posedge clk);

        $display("============================================================");
        $display("ATX-SPEC-020 Accelerator Simulation");
        $display("Goal: validate gates, audit-before-response, and SIMD lane hit path");
        $display("============================================================");

        reset_inputs();
        req_sequence = 32'd1;
        req_key_hash = 32'h0000002A;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("authorized_lane5_hit", STATUS_OK);
        if (resp_resolved_id !== 32'h00002005) begin
            tests_failed = tests_failed + 1;
            $display("FAIL authorized_lane5_hit resolved expected=0x00002005 got=0x%08x", resp_resolved_id);
        end

        reset_inputs();
        req_sequence = 32'd2;
        req_crc_ok = 1'b0;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("crc_mismatch_blocks_probe", STATUS_CRC_MISMATCH);

        reset_inputs();
        req_sequence = 32'd3;
        req_ring_ok = 1'b0;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("ring_denied_blocks_probe", STATUS_RING_DENIED);

        reset_inputs();
        req_sequence = 32'd4;
        req_capability_ok = 1'b0;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("cap_denied_blocks_probe", STATUS_CAP_DENIED);

        reset_inputs();
        req_sequence = 32'd5;
        req_policy_ok = 1'b0;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("policy_denied_blocks_probe", STATUS_POLICY_DENIED);

        reset_inputs();
        req_sequence = 32'd6;
        req_type = 16'h9999;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("unknown_type_rejected", STATUS_INVALID_HEADER);

        reset_inputs();
        req_sequence = 32'd7;
        table_lookup_valid = 1'b0;
        issue_request();
        expect_status("table_timeout", STATUS_TIMEOUT);

        reset_inputs();
        req_sequence = 32'd8;
        req_key_hash = 32'h0000003B;
        set_lane(5, 7'h2A);
        issue_request();
        expect_status("miss_sequence_error", STATUS_SEQUENCE_ERROR);

        $display("============================================================");
        $display("ATX020 SUMMARY tests_run=%0d tests_failed=%0d", tests_run, tests_failed);
        if (tests_failed == 0) begin
            $display("ATX020 RESULT PASS");
        end else begin
            $display("ATX020 RESULT FAIL");
        end
        $display("Expected gain proxy: lane scan checks 16 control bytes in one probe cycle.");
        $display("Future benchmark: compare against scalar 16-step control-byte loop.");
        $display("============================================================");

        #20;
        $finish;
    end

endmodule
