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
    integer hold_count = 0;

    reg [7:0] held_resp_status;
    reg [31:0] held_resp_resolved_id;
    reg [31:0] held_resp_generation;
    reg [3:0] held_resp_offset;
    reg [31:0] held_resp_cycles;
    reg [31:0] held_audit_sequence;
    reg [7:0] held_audit_status;
    reg [31:0] held_audit_registry;
    reg [31:0] held_audit_generation;
    reg [7:0] held_audit_probes;
    reg [3:0] held_audit_offset;

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

    task fail;
        input [8*80-1:0] message;
        begin
            tests_failed = tests_failed + 1;
            $display("FAIL %0s", message);
        end
    endtask

    task clear_inputs;
        begin
            req_valid = 0;
            req_type = 16'h0020;
            req_crc_ok = 1;
            req_ring_ok = 1;
            req_capability_ok = 1;
            req_policy_ok = 1;
            req_registry_id = 32'h00001000;
            req_key_hash = 32'h0000002a;
            req_probe_limit = 8'h04;
            table_control_bytes = 128'h0;
            table_lookup_valid = 1;
            table_generation = 32'h00000005;
            table_base_resolved_id = 32'h00002000;
            resp_ready = 0;
            audit_ready = 1;
        end
    endtask

    task set_lane;
        input integer lane;
        input [6:0] fingerprint;
        begin
            table_control_bytes[(lane * 8) +: 8] = {1'b1, fingerprint};
        end
    endtask

    task check_no_unknowns;
        input [8*40-1:0] name;
        begin
            if ($isunknown({
                resp_status, resp_resolved_id, resp_resolved_generation,
                resp_match_offset, resp_cycles, audit_sequence, audit_status,
                audit_registry_id, audit_generation, audit_probe_count,
                audit_match_offset
            })) begin
                fail({name, ": X/Z on architectural output"});
            end
        end
    endtask

    task run_case;
        input [31:0] seq;
        input [8*40-1:0] name;
        input [7:0] expected_status;
        input [31:0] expected_resolved_id;
        input [31:0] expected_generation;
        input [3:0] expected_offset;
        input [7:0] expected_probes;
        input integer audit_hold_cycles;
        input integer resp_hold_cycles;
        begin
            req_sequence = seq;
            timeout_count = 0;
            audit_ready = (audit_hold_cycles == 0);

            @(posedge clk);
            if (!req_ready)
                fail({name, ": req_ready low before request"});
            req_valid = 1;
            @(posedge clk);
            req_valid = 0;

            while (audit_valid != 1 && timeout_count < 100) begin
                @(posedge clk);
                timeout_count = timeout_count + 1;
            end

            tests_run = tests_run + 1;
            if (audit_valid != 1) begin
                fail({name, ": audit timeout"});
            end else begin
                check_no_unknowns(name);
                if (audit_sequence !== seq) fail({name, ": audit sequence mismatch"});
                if (audit_status !== expected_status) fail({name, ": audit status mismatch"});
                if (audit_registry_id !== req_registry_id) fail({name, ": audit registry mismatch"});
                if (audit_generation !== table_generation) fail({name, ": audit generation mismatch"});
                if (audit_probe_count !== expected_probes) fail({name, ": audit probe count mismatch"});
                if (audit_match_offset !== expected_offset) fail({name, ": audit offset mismatch"});

                held_audit_sequence = audit_sequence;
                held_audit_status = audit_status;
                held_audit_registry = audit_registry_id;
                held_audit_generation = audit_generation;
                held_audit_probes = audit_probe_count;
                held_audit_offset = audit_match_offset;

                for (hold_count = 0; hold_count < audit_hold_cycles; hold_count = hold_count + 1) begin
                    @(posedge clk);
                    if (!audit_valid) fail({name, ": audit_valid dropped under backpressure"});
                    if ({audit_sequence, audit_status, audit_registry_id, audit_generation,
                         audit_probe_count, audit_match_offset} !==
                        {held_audit_sequence, held_audit_status, held_audit_registry,
                         held_audit_generation, held_audit_probes, held_audit_offset})
                        fail({name, ": audit payload changed under backpressure"});
                end

                if (audit_hold_cycles != 0) begin
                    audit_ready = 1;
                    @(posedge clk);
                end
            end

            timeout_count = 0;
            while (resp_valid != 1 && timeout_count < 100) begin
                @(posedge clk);
                timeout_count = timeout_count + 1;
            end

            if (resp_valid != 1) begin
                fail({name, ": response timeout"});
            end else begin
                check_no_unknowns(name);
                if (resp_status !== expected_status) fail({name, ": response status mismatch"});
                if (resp_resolved_id !== expected_resolved_id) fail({name, ": resolved ID mismatch"});
                if (resp_resolved_generation !== expected_generation) fail({name, ": response generation mismatch"});
                if (resp_match_offset !== expected_offset) fail({name, ": response offset mismatch"});
                if (resp_cycles == 0) fail({name, ": zero response cycle count"});

                held_resp_status = resp_status;
                held_resp_resolved_id = resp_resolved_id;
                held_resp_generation = resp_resolved_generation;
                held_resp_offset = resp_match_offset;
                held_resp_cycles = resp_cycles;

                for (hold_count = 0; hold_count < resp_hold_cycles; hold_count = hold_count + 1) begin
                    @(posedge clk);
                    if (!resp_valid) fail({name, ": resp_valid dropped under backpressure"});
                    if ({resp_status, resp_resolved_id, resp_resolved_generation,
                         resp_match_offset, resp_cycles} !==
                        {held_resp_status, held_resp_resolved_id, held_resp_generation,
                         held_resp_offset, held_resp_cycles})
                        fail({name, ": response payload changed under backpressure"});
                end

                $display("PASS %-32s status=0x%02x probes=%0d offset=%0d resolved=0x%08x",
                         name, resp_status, audit_probe_count, audit_match_offset,
                         resp_resolved_id);
            end

            resp_ready = 1;
            @(posedge clk);
            @(posedge clk);
            resp_ready = 0;
            audit_ready = 1;
            repeat (2) @(posedge clk);
        end
    endtask

    task check_idle_reset;
        input [8*40-1:0] name;
        begin
            rst_n = 0;
            repeat (2) @(posedge clk);
            if (req_ready !== 1'b1) fail({name, ": req_ready not asserted in reset"});
            if (resp_valid !== 1'b0) fail({name, ": spurious response in reset"});
            if (audit_valid !== 1'b0) fail({name, ": spurious audit in reset"});
            rst_n = 1;
            repeat (2) @(posedge clk);
            if (req_ready !== 1'b1) fail({name, ": req_ready not restored after reset"});
            $display("PASS %-32s", name);
            tests_run = tests_run + 1;
        end
    endtask

    initial begin
        $dumpfile("atx_spec_020_accelerator_tb.vcd");
        $dumpvars(0, atx_spec_020_accelerator_ci_tb);

        repeat (4) @(posedge clk);
        rst_n = 1;
        repeat (2) @(posedge clk);
        $display("ATX-SPEC-020 CI Accelerator Simulation");

        clear_inputs(); set_lane(5, 7'h2a);
        run_case(1, "authorized_lane5_hit", 8'h00, 32'h00002005, 32'h00000005, 4'd5, 8'd1, 0, 3);

        clear_inputs(); set_lane(0, 7'h2a);
        run_case(2, "authorized_lane0_boundary", 8'h00, 32'h00002000, 32'h00000005, 4'd0, 8'd1, 0, 1);

        clear_inputs(); set_lane(15, 7'h2a);
        run_case(3, "authorized_lane15_boundary", 8'h00, 32'h0000200f, 32'h00000005, 4'd15, 8'd1, 2, 1);

        clear_inputs(); set_lane(3, 7'h2a); set_lane(11, 7'h2a);
        run_case(4, "multiple_match_lowest_lane", 8'h00, 32'h00002003, 32'h00000005, 4'd3, 8'd1, 0, 0);

        clear_inputs(); req_crc_ok = 0; set_lane(5, 7'h2a);
        run_case(5, "crc_mismatch", 8'h04, 32'hffffffff, 32'h00000000, 4'd0, 8'd0, 0, 0);

        clear_inputs(); req_ring_ok = 0; set_lane(5, 7'h2a);
        run_case(6, "ring_denied", 8'h09, 32'hffffffff, 32'h00000000, 4'd0, 8'd0, 0, 0);

        clear_inputs(); req_capability_ok = 0; set_lane(5, 7'h2a);
        run_case(7, "cap_denied", 8'h0b, 32'hffffffff, 32'h00000000, 4'd0, 8'd0, 0, 0);

        clear_inputs(); req_policy_ok = 0; set_lane(5, 7'h2a);
        run_case(8, "policy_denied", 8'h0a, 32'hffffffff, 32'h00000000, 4'd0, 8'd0, 0, 0);

        clear_inputs(); req_type = 16'h9999; set_lane(5, 7'h2a);
        run_case(9, "unknown_type", 8'h02, 32'hffffffff, 32'h00000000, 4'd0, 8'd0, 0, 0);

        clear_inputs(); table_lookup_valid = 0;
        run_case(10, "table_timeout", 8'h0e, 32'hffffffff, 32'h00000000, 4'd0, 8'd0, 0, 0);

        clear_inputs(); req_key_hash = 32'h0000003b; set_lane(5, 7'h2a);
        run_case(11, "miss_sequence_error", 8'h05, 32'hffffffff, 32'h00000000, 4'd0, 8'd5, 0, 0);

        check_idle_reset("idle_reset_contract");

        $display("ATX020 SUMMARY tests_run=%0d tests_failed=%0d", tests_run, tests_failed);
        if (tests_failed == 0) $display("ATX020 RESULT PASS");
        else $display("ATX020 RESULT FAIL");
        $display("Expected gain proxy: lane scan checks 16 control bytes in one probe cycle.");
        #20;
        $finish;
    end
endmodule
