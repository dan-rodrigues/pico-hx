// usb_programmer.c
//
// Copyright (C) 2021 Dan Rodrigues <danrr.gh.oss@gmail.com>
//
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "usb_programmer.h"
#include "fpga_spi.h"

enum phx_ctrl_req {
	PHX_CTRL_REQ_PREPARE_BITSTREAM_LOAD = 0x01,
	PHX_CTRL_REQ_FINALIZE_BITSTREAM_LOAD = 0x02
};

// Preload bitstream for simplicity
// Could alternatively stream it assuming no interruptions
static uint8_t bitstream[0x8000];
static size_t bitstream_index;

void usb_prog_task() {
	if (!tud_vendor_available()) {
		return;
	}

	if (bitstream_index >= sizeof(bitstream)) {
		printf("%s: data available but buffer is exhausted\n", __FUNCTION__);
		return;
	}

	const size_t packet_size = 64;
    uint32_t rx_len = tud_vendor_read(&bitstream[bitstream_index], packet_size);
    if (rx_len == 0) {
        return;
    }

    bitstream_index += rx_len;
}

bool tud_vendor_control_request_cb(uint8_t rhport, tusb_control_request_t const *request) {
    // These control requests don't have a data phase so just immediately act on them
    switch (request->bRequest) {
        case PHX_CTRL_REQ_PREPARE_BITSTREAM_LOAD:
            bitstream_index = 0;
            return tud_control_status(rhport, request);
        case PHX_CTRL_REQ_FINALIZE_BITSTREAM_LOAD:
            fpga_upload_bitstream(bitstream, bitstream_index);
            return tud_control_status(rhport, request);
        default:
            printf("%s: unknown ctrl request: %x\n", __FUNCTION__, request->bRequest);
            return false;
    }
}
