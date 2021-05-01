// picohx_demo_top.v
//
// Copyright (C) 2021 Dan Rodrigues <danrr.gh.oss@gmail.com>
//
// SPDX-License-Identifier: CERN-OHL-W-2.0

`default_nettype none

module picohx_demo_top (
    input clk_in,

    output [7:0] led,

    input btn_a,
    input btn_b
);
    // --- PLL ---

    wire clk_12m = clk_in;
    wire clk_24m;
    wire pll_locked;

    SB_PLL40_2F_PAD #(
        .DIVR(4'b0000),
        .DIVF(7'b0111111),
        .DIVQ(3'b101),
        .FILTER_RANGE(3'b001),
        .FEEDBACK_PATH("SIMPLE"),
        .DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
        .FDA_FEEDBACK(4'b0000),
        .DELAY_ADJUSTMENT_MODE_RELATIVE("FIXED"),
        .FDA_RELATIVE(4'b0000),
        .SHIFTREG_DIV_MODE(2'b00),
        .PLLOUT_SELECT_PORTA("GENCLK"),
    ) pll (
        .PACKAGEPIN(clk_12m),
        .PLLOUTGLOBALA(clk_24m),
        .LOCK(pll_locked),
        .EXTFEEDBACK(),
        .DYNAMICDELAY(),
        .RESETB(1'b1),
        .BYPASS(1'b0),
        .LATCHINPUTVALUE()
    );

    wire reset = !reset_counter[3];
    reg [3:0] reset_counter = 0;

    always @(posedge clk_24m) begin
        if (!pll_locked) begin
            reset_counter <= 0;
        end else if (reset) begin
            reset_counter <= reset_counter + 1;
        end
    end

    // --- LED ---

    reg [31:0] counter;
    reg count_enabled;

    assign led[7:0] = counter[29:22];

    always @(posedge clk_24m) begin
        if (count_enabled) begin
            counter <= counter + 1;
        end else if (btn_a_trigger) begin
            counter = counter >> 1;
        end
    end

    always @(posedge clk_24m) begin
        if (reset) begin
            count_enabled <= 0;
        end else if (btn_b_trigger) begin
            count_enabled <= !count_enabled;
        end
    end

    // --- Buttons ---

    wire btn_a_level;
    wire btn_b_level;
    wire btn_a_trigger;
    wire btn_b_trigger;

    debouncer #(
        .BTN_COUNT(4)
    ) debouncer (
        .clk(clk_24m),
        .reset(reset),

        .btn({btn_b, btn_a}),
        .level({btn_b_level, btn_a_level}),
        .trigger({btn_a_trigger, btn_b_trigger})
    );

endmodule
