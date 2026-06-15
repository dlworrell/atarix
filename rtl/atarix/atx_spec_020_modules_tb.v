`timescale 1ns / 1ps

module atx_spec_020_modules_tb;

    reg clk;
    reg rst_n;

    reg [127:0] control_bytes;
    reg [6:0] target_h2;
    wire simd_match;
    wire [3:0] simd_offset;
    wire [15:0] simd_lanes;

    reg scalar_start;
    wire scalar_done;
    wire scalar_match;
    wire [3:0] scalar_offset;
    wire [4:0] scalar_cycles;

    reg [31:0] step_base;
    reg [7:0] step_attempt;
    wire [31:0] step_next;
    wire [31:0] step_offset;

    reg [31:0] ef_high;
    reg [31:0] ef_low;
    reg [3:0] ef_target;
    reg [4:0] ef_low_bits;
    wire [31:0] ef_row;
    wire ef_valid;
    wire ef_error;

    reg [7:0] audit_write_data;
    reg audit_write_en;
    reg [7:0] audit_read_addr;
    wire [7:0] audit_read_data;
    wire [7:0] audit_head;
    wire audit_wrapped;

    integer tests_failed;

    atx_simd_probe_core simd (
        .control_bytes(control_bytes),
        .target_h2(target_h2),
        .match_found(simd_match),
        .match_offset(simd_offset),
        .lane_matches(simd_lanes)
    );

    atx_scalar_probe_core scalar (
        .clk(clk),
        .rst_n(rst_n),
        .start(scalar_start),
        .control_bytes(control_bytes),
        .target_h2(target_h2),
        .done(scalar_done),
        .match_found(scalar_match),
        .match_offset(scalar_offset),
        .cycles_used(scalar_cycles)
    );

    atx_krapivin_stepper stepper (
        .current_base(step_base),
        .current_attempt(step_attempt),
        .table_mask(32'h0000FFFF),
        .next_base(step_next),
        .non_linear_offset(step_offset)
    );

    atx_elias_fano_decoder ef (
        .high_bits_word(ef_high),
        .low_bits_word(ef_low),
        .target_offset(ef_target),
        .num_low_bits(ef_low_bits),
        .resolved_row_id(ef_row),
        .decode_valid(ef_valid),
        .decode_error(ef_error)
    );

    atx_audit_log_window audit (
        .clk(clk),
        .rst_n(rst_n),
        .write_data(audit_write_data),
        .write_en(audit_write_en),
        .read_addr(audit_read_addr),
        .read_data(audit_read_data),
        .head_pointer(audit_head),
        .wrapped(audit_wrapped)
    );

    always #5 clk = ~clk;

    task set_lane;
        input integer lane;
        input [6:0] fingerprint;
        begin
            control_bytes[(lane * 8) +: 8] = {1'b1, fingerprint};
        end
    endtask

    task fail;
        input [8*64-1:0] name;
        begin
            tests_failed = tests_failed + 1;
            $display("FAIL %s", name);
        end
    endtask

    initial begin
        $dumpfile("atx_spec_020_modules_tb.vcd");
        $dumpvars(0, atx_spec_020_modules_tb);

        clk = 1'b0;
        rst_n = 1'b0;
        tests_failed = 0;
        control_bytes = 128'h0;
        target_h2 = 7'h2A;
        scalar_start = 1'b0;
        step_base = 32'h00001000;
        step_attempt = 8'd3;
        ef_high = 32'b0000_0000_0000_0000_0000_0000_0010_1010;
        ef_low = 32'h000000E4;
        ef_target = 4'd2;
        ef_low_bits = 5'd2;
        audit_write_data = 8'h00;
        audit_write_en = 1'b0;
        audit_read_addr = 8'h00;

        repeat (3) @(posedge clk);
        rst_n = 1'b1;
        repeat (2) @(posedge clk);

        $display("ATX020 MODULES: begin");

        set_lane(9, 7'h2A);
        #1;
        if (!simd_match || simd_offset != 4'd9) fail("simd lane9 match");
        else $display("PASS simd lane9 match offset=%0d lanes=0x%04x", simd_offset, simd_lanes);

        scalar_start = 1'b1;
        @(posedge clk);
        scalar_start = 1'b0;
        wait(scalar_done == 1'b1);
        if (!scalar_match || scalar_offset != 4'd9) fail("scalar lane9 match");
        else $display("PASS scalar lane9 match offset=%0d cycles=%0d", scalar_offset, scalar_cycles);

        if (scalar_cycles <= 5'd1) fail("scalar should take multiple cycles");
        else $display("PASS gain proxy simd=1-cycle combinational scalar_cycles=%0d", scalar_cycles);

        #1;
        if (step_offset != 32'd6 || step_next != 32'h00001006) fail("krapivin step attempt3");
        else $display("PASS krapivin step offset=%0d next=0x%08x", step_offset, step_next);

        #1;
        if (!ef_valid || ef_error) fail("elias fano valid decode");
        else $display("PASS elias-fano row=%0d", ef_row);

        audit_write_data = 8'h41;
        audit_write_en = 1'b1;
        @(posedge clk);
        audit_write_en = 1'b0;
        repeat (1) @(posedge clk);
        audit_read_addr = 8'h00;
        #1;
        if (audit_read_data != 8'h41) fail("audit readback");
        else $display("PASS audit readback data=%c head=%0d", audit_read_data, audit_head);

        if (tests_failed == 0) $display("ATX020 MODULES RESULT PASS");
        else $display("ATX020 MODULES RESULT FAIL failures=%0d", tests_failed);

        #20;
        $finish;
    end

endmodule
