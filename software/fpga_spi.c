// fpga_spi.c
//
// Copyright (C) 2021 Dan Rodrigues <danrr.gh.oss@gmail.com>
//
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "fpga_spi.h"
#include "phx_gpio.h"

#include "spi.pio.h"

void __not_in_flash_func(fpga_upload_bitstream)(const uint8_t *bitstream, size_t len) {
    uint cdone = PHX_GPIO_FPGA_CDONE;
    uint creset = PHX_GPIO_FPGA_CRESET;
    uint sck = PHX_GPIO_FPGA_SCK;
    uint sdo = PHX_GPIO_FPGA_SDO;
    uint sdi = PHX_GPIO_FPGA_SDI;
    uint ss = PHX_GPIO_FPGA_SS;

    // Init PIO for bistream sending
    PIO pio = pio0;

    // 1 of 4 SMs must be claimed
    uint sm = pio_claim_unused_sm(pio, true);
    // 25MHz, the max speed specified for iCE40 SPI
    float div = 125 / 25.0;
    fpga_spi_config_init(pio, sm, sdi, sck, div);

    // CDONE pullup
    gpio_init(cdone);
    gpio_set_dir(cdone, GPIO_IN);
    gpio_pull_up(cdone);

    // Assert SSB
    gpio_init(ss);
    gpio_put(ss, 0);
    gpio_set_dir(ss, GPIO_OUT);

    // Assert CRESETB for at least 200us
    gpio_init(creset);
    gpio_put(creset, 0);
    gpio_set_dir(creset, GPIO_OUT);
    busy_wait_us_32(300);
    // ..deassert CRESETB
    gpio_put(creset, 1);

    // 1200uS wait (CRAM clear)
    busy_wait_us_32(2000);

    // Deassert SS, send 8 clocks
    gpio_put(ss, 1);
    busy_wait_us_32(2);

    fpga_spi_config_put(pio0, 0, 0);
    fpga_spi_config_flush_fifo(pio0, 0);

    // ..then reassert
    gpio_put(ss, 0);

    // Send bitstream
    for (size_t i = 0; i < len; i++)
        fpga_spi_config_put(pio0, sm, bitstream[i]);

    // Send at least 100 clocks immediately after bitstream
    const uint dummy_bytes = 100 / 8 + 1;
    for (int i = 0; i < dummy_bytes; i++)
        fpga_spi_config_put(pio0, sm, 0);

    fpga_spi_config_flush_fifo(pio0, sm);

    // All PIO work is done now
    pio_sm_unclaim(pio, sm);

    // Check CDONE status to confirm successful config
    if (gpio_get(cdone)) {
        printf("%s: successfully programmed\n", __FUNCTION__);

        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    } else {
        printf("%s: failed to program\n", __FUNCTION__);

        while (true) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(200);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            sleep_ms(200);
        }
    }
}
