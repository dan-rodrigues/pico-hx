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
    input btn_b,

    // PMOD-C

    output pmod_c1,
    output pmod_c2,
    output pmod_c3,
    output pmod_c4,

    output pmod_c7,
    output pmod_c8,
    output pmod_c9,
    output pmod_c10,

    // PMOD-D

    output pmod_d1,
    output pmod_d2,
    output pmod_d3,
    output pmod_d4,

    output pmod_d7,
    output pmod_d8,
    output pmod_d9,
    output pmod_d10
);
    // --- PLL ---

    wire clk_12m = clk_in;
    wire clk_20m;
    wire clk_40m;
    wire pll_locked;

    SB_PLL40_2F_PAD #(
        .DIVR(4'b0000),
        .DIVF(7'b0110100),
        .DIVQ(3'b100),
        .FILTER_RANGE(3'b001),
        .FEEDBACK_PATH("SIMPLE"),
        .DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
        .FDA_FEEDBACK(4'b0000),
        .DELAY_ADJUSTMENT_MODE_RELATIVE("FIXED"),
        .FDA_RELATIVE(4'b0000),
        .SHIFTREG_DIV_MODE(2'b00),
        .PLLOUT_SELECT_PORTA("GENCLK"),
        .PLLOUT_SELECT_PORTB("GENCLK_HALF")
    ) pll (
        .PACKAGEPIN(clk_12m),
        .PLLOUTGLOBALA(clk_40m),
        .PLLOUTGLOBALB(clk_20m),
        .LOCK(pll_locked),
        .EXTFEEDBACK(),
        .DYNAMICDELAY(),
        .RESETB(1'b1),
        .BYPASS(1'b0),
        .LATCHINPUTVALUE()
    );

    wire reset = !reset_counter[3];
    reg [3:0] reset_counter = 0;

    always @(posedge clk_20m) begin
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

    always @(posedge clk_20m) begin
        if (count_enabled) begin
            counter <= counter + 1;
        end else if (btn_a_trigger) begin
            counter = counter >> 1;
        end
    end

    always @(posedge clk_20m) begin
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
        .clk(clk_20m),
        .reset(reset),

        .btn({btn_b, btn_a}),
        .level({btn_b_level, btn_a_level}),
        .trigger({btn_a_trigger, btn_b_trigger})
    );

    reg video_mode;

    always @(posedge clk_20m) begin
        if (reset) begin
            video_mode <= 0;
        end else if (btn_a_trigger) begin
            video_mode <= !video_mode;
        end
    end

    // --- Video ---

    wire vga_de;
    wire vga_ck;
    wire vga_hs;
    wire vga_vs;
    wire [23:0] vga_rgb;
    wire [7:0] r;
    wire [7:0] g;
    wire [7:0] b;

    vga_core vga_core(
        .clk_dot(clk_40m),
        .reset(reset),

        .random_num(0 /*random_num[31:0]*/),
        .color_3b(1'b0),
        .mode_bit(video_mode),
        .vga_active(vga_de),
        .vga_hsync(vga_hs),
        .vga_vsync(vga_vs),
        .vga_pixel_rgb(vga_rgb[23:0])
    );

    assign r = vga_rgb[23:16];
    assign g = vga_rgb[15:8];
    assign b = vga_rgb[7:0];

    // PMOD assignment:

    assign vga_ck = clk_40m;

    // 12b for dual-PMOD:

    assign {
        pmod_c1, pmod_c2, pmod_c3, pmod_c4, pmod_c7, pmod_c8, pmod_c9, pmod_c10
    } = {r[7],  r[5], g[7], g[5], r[6], r[4], g[6], g[4]};

    assign {
        pmod_d1, pmod_d2, pmod_d3, pmod_d4, pmod_d7, pmod_d8, pmod_d9, pmod_d10
    } =  {b[7], vga_ck, b[4], vga_hs, b[6], b[5], vga_de, vga_vs};

endmodule
