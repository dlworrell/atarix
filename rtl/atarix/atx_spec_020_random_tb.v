`timescale 1ns / 1ps

module atx_spec_020_random_tb;
    localparam CASES = 64;
    localparam STATUS_OK             = 8'h00;
    localparam STATUS_INVALID_HEADER = 8'h02;
    localparam STATUS_CRC_MISMATCH   = 8'h04;
    localparam STATUS_SEQUENCE_ERROR = 8'h05;
    localparam STATUS_RING_DENIED    = 8'h09;
    localparam STATUS_POLICY_DENIED  = 8'h0a;
    localparam STATUS_CAP_DENIED     = 8'h0b;
    localparam STATUS_TIMEOUT        = 8'h0e;

    reg clk = 0;
    reg rst_n = 0;
    reg req_valid = 0;
    wire req_ready;
    reg [31:0] req_sequence = 0;
    reg [15:0] req_type = 16'h0020;
    reg [15:0] req_source_ring = 1;
    reg [15:0] req_target_ring = 2;
    reg req_crc_ok = 1;
    reg req_ring_ok = 1;
    reg req_capability_ok = 1;
    reg req_policy_ok = 1;
    reg [31:0] req_registry_id = 32'h1000;
    reg [31:0] req_key_hash = 0;
    reg [31:0] req_expected_generation = 5;
    reg [7:0] req_query_type = 0;
    reg [7:0] req_probe_limit = 4;
    reg [7:0] req_projection_flags = 3;
    reg [127:0] table_control_bytes = 0;
    reg [31:0] table_generation = 5;
    reg [31:0] table_base_resolved_id = 32'h2000;
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
    reg audit_ready = 0;
    wire [31:0] audit_sequence;
    wire [7:0] audit_status;
    wire [31:0] audit_registry_id;
    wire [31:0] audit_generation;
    wire [7:0] audit_probe_count;
    wire [3:0] audit_match_offset;

    integer seed;
    integer i;
    integer lane;
    integer tests_failed = 0;
    integer timeout_count;
    integer audit_hold;
    integer resp_hold;
    integer lowest_lane;
    integer match_count;
    reg [7:0] expected_status;
    reg [31:0] expected_resolved;
    reg [31:0] expected_generation;
    reg [3:0] expected_offset;
    reg [7:0] expected_probes;
    reg [6:0] fingerprint;
    reg [31:0] rv;
    reg [31:0] held_resp_cycles;

    atx_spec_020_accelerator dut (
        .clk(clk), .rst_n(rst_n), .req_valid(req_valid), .req_ready(req_ready),
        .req_sequence(req_sequence), .req_type(req_type),
        .req_source_ring(req_source_ring), .req_target_ring(req_target_ring),
        .req_crc_ok(req_crc_ok), .req_ring_ok(req_ring_ok),
        .req_capability_ok(req_capability_ok), .req_policy_ok(req_policy_ok),
        .req_registry_id(req_registry_id), .req_key_hash(req_key_hash),
        .req_expected_generation(req_expected_generation),
        .req_query_type(req_query_type), .req_probe_limit(req_probe_limit),
        .req_projection_flags(req_projection_flags),
        .table_control_bytes(table_control_bytes), .table_generation(table_generation),
        .table_base_resolved_id(table_base_resolved_id),
        .table_lookup_valid(table_lookup_valid), .probe_valid(probe_valid),
        .probe_addr(probe_addr), .probe_next_addr(probe_next_addr),
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

    task fail_case;
        input [8*80-1:0] reason;
        begin
            tests_failed = tests_failed + 1;
            $display("FAIL random case=%0d seed=%0d reason=%0s type=%04x crc=%0d ring=%0d cap=%0d policy=%0d lookup=%0d limit=%0d matches=%0d lowest=%0d status=%02x",
                     i, seed, reason, req_type, req_crc_ok, req_ring_ok,
                     req_capability_ok, req_policy_ok, table_lookup_valid,
                     req_probe_limit, match_count, lowest_lane, resp_status);
        end
    endtask

    task set_lane;
        input integer lane_number;
        input [6:0] lane_fingerprint;
        begin
            table_control_bytes[(lane_number * 8) +: 8] = {1'b1, lane_fingerprint};
        end
    endtask

    task build_expected;
        begin
            expected_status = STATUS_OK;
            expected_resolved = 32'hffffffff;
            expected_generation = 0;
            expected_offset = 0;
            expected_probes = 0;
            if (req_type != 16'h0020)
                expected_status = STATUS_INVALID_HEADER;
            else if (!req_crc_ok)
                expected_status = STATUS_CRC_MISMATCH;
            else if (!req_ring_ok)
                expected_status = STATUS_RING_DENIED;
            else if (!req_capability_ok)
                expected_status = STATUS_CAP_DENIED;
            else if (!req_policy_ok)
                expected_status = STATUS_POLICY_DENIED;
            else if (!table_lookup_valid)
                expected_status = STATUS_TIMEOUT;
            else if (lowest_lane >= 0) begin
                expected_status = STATUS_OK;
                expected_offset = lowest_lane[3:0];
                expected_resolved = table_base_resolved_id + lowest_lane;
                expected_generation = table_generation;
                expected_probes = 1;
            end else begin
                expected_status = STATUS_SEQUENCE_ERROR;
                expected_probes = req_probe_limit + 1'b1;
            end
        end
    endtask

    initial begin
        seed = 32'h02002026;
        if ($value$plusargs("SEED=%d", seed)) begin end
        $display("ATX-SPEC-020 RANDOM seed=%0d cases=%0d", seed, CASES);
        repeat (4) @(posedge clk);
        rst_n = 1;
        repeat (2) @(posedge clk);

        for (i = 0; i < CASES; i = i + 1) begin
            rv = $urandom(seed);
            req_sequence = 32'h80000000 + i;
            req_type = ((rv % 13) == 0) ? 16'h9999 : 16'h0020;
            req_crc_ok = ((rv >> 4) % 11) != 0;
            req_ring_ok = ((rv >> 8) % 9) != 0;
            req_capability_ok = ((rv >> 12) % 8) != 0;
            req_policy_ok = ((rv >> 16) % 7) != 0;
            table_lookup_valid = ((rv >> 20) % 10) != 0;
            req_probe_limit = (rv >> 24) & 8'h0f;
            req_registry_id = 32'h1000 + i;
            table_generation = 32'h5000 + i;
            table_base_resolved_id = 32'h20000000 + (i * 32);
            fingerprint = rv[6:0];
            req_key_hash = {25'h0, fingerprint};
            table_control_bytes = 0;
            lowest_lane = -1;
            match_count = 0;

            for (lane = 0; lane < 16; lane = lane + 1) begin
                rv = $urandom(seed);
                if ((rv & 3) == 0) begin
                    set_lane(lane, fingerprint);
                    match_count = match_count + 1;
                    if (lowest_lane < 0) lowest_lane = lane;
                end else if ((rv & 7) == 1) begin
                    set_lane(lane, rv[6:0] ^ 7'h55);
                end
            end

            build_expected();
            audit_hold = $urandom(seed) % 4;
            resp_hold = $urandom(seed) % 4;
            audit_ready = 0;
            resp_ready = 0;

            @(posedge clk);
            if (!req_ready) fail_case("req_ready low");
            req_valid = 1;
            @(posedge clk);
            req_valid = 0;

            timeout_count = 0;
            while (!audit_valid && timeout_count < 128) begin
                @(posedge clk);
                timeout_count = timeout_count + 1;
            end
            if (!audit_valid) begin
                fail_case("audit timeout");
            end else begin
                if ({audit_sequence, audit_status, audit_registry_id, audit_generation,
                     audit_probe_count, audit_match_offset} !==
                    {req_sequence, expected_status, req_registry_id, table_generation,
                     expected_probes, expected_offset})
                    fail_case("audit mismatch");
                repeat (audit_hold) begin
                    @(posedge clk);
                    if (!audit_valid) fail_case("audit dropped under backpressure");
                end
                audit_ready = 1;
                @(posedge clk);
                audit_ready = 0;
            end

            timeout_count = 0;
            while (!resp_valid && timeout_count < 128) begin
                @(posedge clk);
                timeout_count = timeout_count + 1;
            end
            if (!resp_valid) begin
                fail_case("response timeout");
            end else begin
                if ({resp_status, resp_resolved_id, resp_resolved_generation,
                     resp_match_offset} !==
                    {expected_status, expected_resolved, expected_generation,
                     expected_offset})
                    fail_case("response mismatch");
                if ($isunknown({resp_status, resp_resolved_id,
                                resp_resolved_generation, resp_match_offset,
                                resp_cycles}))
                    fail_case("response contains X/Z");
                held_resp_cycles = resp_cycles;
                repeat (resp_hold) begin
                    @(posedge clk);
                    if (!resp_valid) fail_case("response dropped under backpressure");
                    if (resp_cycles !== held_resp_cycles)
                        fail_case("response cycles changed under backpressure");
                end
                resp_ready = 1;
                @(posedge clk);
                @(posedge clk);
                resp_ready = 0;
            end
            repeat (2) @(posedge clk);
        end

        $display("ATX020 RANDOM SUMMARY cases=%0d failed=%0d seed=%0d", CASES, tests_failed, seed);
        if (tests_failed == 0) $display("ATX020 RANDOM RESULT PASS");
        else $display("ATX020 RANDOM RESULT FAIL");
        #20;
        $finish;
    end
endmodule
