// main.c
//
// Copyright (C) 2021 Dan Rodrigues <danrr.gh.oss@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "phx_gpio.h"
#include "tusb.h"
#include "get_serial.h"
#include "usb_uart.h"
#include "usb_programmer.h"
#include "fpga_spi.h"

#include "default_bitstream.h"

int main() {
#ifndef PICO_DEFAULT_LED_PIN
#warning blink example requires a board with a regular LED
#else
    // Default to Pico LED on
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    // 48MHz / 4 = 12MHz clock output to FPGA
    const uint usb_clk = 48;
    const uint target_clk = 12;
    const uint clk_divisor = usb_clk / target_clk;
    clock_gpio_init(
        PHX_GPIO_FPGA_CLK_IN,
        CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
        clk_divisor
    );

    // Program the included default bitstream
    // This can be replaced later at any time with the USB programmer script
    fpga_upload_bitstream(default_bitstream, default_bitstream_len);

    // USB init before entering main loop
    tusb_init();
    stdio_usb_init();

    while (true) {
        tud_task();
        usb_prog_task();
    }
#endif
}
