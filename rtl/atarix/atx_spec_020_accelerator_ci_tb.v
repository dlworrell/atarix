`timescale 1ns / 1ps

module atx_spec_020_accelerator_ci_tb;
    reg clk = 0;
    reg rst_n = 0;
    reg req_valid = 0;
    wire req_ready;
    reg [31:0] req_sequence = 0;
    reg [15:0] req_type = 16'h0020;
    reg [15:0] req_source_ring = 16'h0001;
    reg [15:0] req_target_ring = 16'h0002;
    reg req_crc_ok = 1;
    reg req_ring_ok = 1;
    reg req_capability_ok = 1;
    reg req_policy_ok = 1;
    reg [31:0] req_registry_id = 32'h00001000;
    reg [31:0] req_key_hash = 32'h0000002a;
    reg [31:0] req_expected_generation = 32'h00000005;
    reg [7:0] req_query_type = 8'h00;
    reg [7:0] req_probe_limit = 8'h04;
    reg [7:0] req_projection_flags = 8'h03;
    reg [127:0] table_control_bytes = 128'h0;
    reg [31:0] table_generation = 32'h00000005;
    reg [31:0] table_base_resolved_id = 32'h00002000;
    reg table_lookup_valid = 1;
    wire probe_valid;
    wire [31:0] probe_addr;
    wire [31:0] probe_next_addr;
    wire resp_valid;
    reg resp_ready = 0;
    wire [7:0] resp_status;
    wire [31:0] resp_resolved_id;
    wire [31:0] resp_resolved_generation;
    wire [3:0] resp_match_offset;
    wire [31:0] resp_cycles;
    wire audit_valid;
    reg audit_ready = 1;
    wire [31:0] audit_sequence;
    wire [7:0] audit_status;
    wire [31:0] audit_registry_id;
    wire [31:0] audit_generation;
    wire [7:0] audit_probe_count;
    wire [3:0] audit_match_offset;

    integer tests_run = 0;
    integer tests_failed = 0;
    integer timeout_count = 0;

    atx_spec_020_accelerator dut (
        .clk(clk), .rst_n(rst_n),
        .req_valid(req_valid), .req_ready(req_ready),
        .req_sequence(req_sequence), .req_type(req_type),
        .req_source_ring(req_source_ring), .req_target_ring(req_target_ring),
        .req_crc_ok(req_crc_ok), .req_ring_ok(req_ring_ok),
        .req_capability_ok(req_capability_ok), .req_policy_ok(req_policy_ok),
        .req_registry_id(req_registry_id), .req_key_hash(req_key_hash),
        .req_expected_generation(req_expected_generation),
        .req_query_type(req_query_type), .req_probe_limit(req_probe_limit),
        .req_projection_flags(req_projection_flags),
        .table_control_bytes(table_control_bytes),
        .table_generation(table_generation),
        .table_base_resolved_id(table_base_resolved_id),
        .table_lookup_valid(table_lookup_valid),
        .probe_valid(probe_valid), .probe_addr(probe_addr),
        .probe_next_addr(probe_next_addr),
        .resp_valid(resp_valid), .resp_ready(resp_ready),
        .resp_status(resp_status), .resp_resolved_id(resp_resolved_id),
        .resp_resolved_generation(resp_resolved_generation),
        .resp_match_offset(resp_match_offset), .resp_cycles(resp_cycles),
        .audit_valid(audit_valid), .audit_ready(audit_ready),
        .audit_sequence(audit_sequence), .audit_status(audit_status),
        .audit_registry_id(audit_registry_id), .audit_generation(audit_generation),
        .audit_probe_count(audit_probe_count), .audit_match_offset(audit_match_offset)
    );

    always #5 clk = ~clk;

    task clear_inputs;
        begin
            req_valid = 0; req_type = 16'h0020;
            req_crc_ok = 1; req_ring_ok = 1; req_capability_ok = 1; req_policy_ok = 1;
            req_key_hash = 32'h0000002a; req_probe_limit = 8'h04;
            table_control_bytes = 128'h0; table_lookup_valid = 1;
            table_generation = 32'h00000005; table_base_resolved_id = 32'h00002000;
            resp_ready = 0; audit_ready = 1;
        end
    endtask

    task set_lane;
        input integer lane;
        input [6:0] fingerprint;
        begin
            table_control_bytes[(lane * 8) +: 8] = {1'b1, fingerprint};
        end
    endtask

    task run_case;
        input [31:0] seq;
        input [8*40-1:0] name;
        input [7:0] expected;
        begin
            req_sequence = seq;
            timeout_count = 0;
            @(posedge clk);
            req_valid = 1;
            @(posedge clk);
            req_valid = 0;
            while (resp_valid != 1 && timeout_count < 100) begin
                @(posedge clk);
                timeout_count = timeout_count + 1;
            end
            tests_run = tests_run + 1;
            if (resp_valid != 1) begin
                tests_failed = tests_failed + 1;
                $display("FAIL %-32s timeout", name);
            end else if (resp_status !== expected) begin
                tests_failed = tests_failed + 1;
                $display("FAIL %-32s expected=0x%02x got=0x%02x", name, expected, resp_status);
            end else begin
                $display("PASS %-32s status=0x%02x probes=%0d offset=%0d resolved=0x%08x", name, resp_status, audit_probe_count, audit_match_offset, resp_resolved_id);
            end
            resp_ready = 1;
            @(posedge clk);
            resp_ready = 0;
            repeat (2) @(posedge clk);
        end
    endtask

    initial begin
        $dumpfile("atx_spec_020_accelerator_tb.vcd");
        $dumpvars(0, atx_spec_020_accelerator_ci_tb);
        repeat (4) @(posedge clk);
        rst_n = 1;
        repeat (2) @(posedge clk);
        $display("ATX-SPEC-020 CI Accelerator Simulation");

        clear_inputs(); set_lane(5, 7'h2a); run_case(1, "authorized_lane5_hit", 8'h00);
        clear_inputs(); req_crc_ok = 0; set_lane(5, 7'h2a); run_case(2, "crc_mismatch", 8'h04);
        clear_inputs(); req_ring_ok = 0; set_lane(5, 7'h2a); run_case(3, "ring_denied", 8'h09);
        clear_inputs(); req_capability_ok = 0; set_lane(5, 7'h2a); run_case(4, "cap_denied", 8'h0b);
        clear_inputs(); req_policy_ok = 0; set_lane(5, 7'h2a); run_case(5, "policy_denied", 8'h0a);
        clear_inputs(); req_type = 16'h9999; set_lane(5, 7'h2a); run_case(6, "unknown_type", 8'h02);
        clear_inputs(); table_lookup_valid = 0; run_case(7, "table_timeout", 8'h0e);
        clear_inputs(); req_key_hash = 32'h0000003b; set_lane(5, 7'h2a); run_case(8, "miss_sequence_error", 8'h05);

        $display("ATX020 SUMMARY tests_run=%0d tests_failed=%0d", tests_run, tests_failed);
        if (tests_failed == 0) $display("ATX020 RESULT PASS");
        else $display("ATX020 RESULT FAIL");
        $display("Expected gain proxy: lane scan checks 16 control bytes in one probe cycle.");
        #20;
        $finish;
    end
endmodule
