#!/usr/bin/env python3

# picoprog.py
#
# Copyright (C) 2021 Dan Rodrigues <danrr.gh.oss@gmail.com>
#
# SPDX-License-Identifier: MIT

import sys

import usb.core
import usb.util

# Expecting 1 argument with bitstream to load
if len(sys.argv) != 2:
    print("Usage: picoprog.py <bistream>")
    sys.exit(0)

# Find Pico programmer device
dev = usb.core.find(idVendor=0x2E8A, idProduct=0x0004)
if dev is None:
    raise RuntimeError("Device not found")

cfg = dev.get_active_configuration()
intf = cfg[(2, 0)]

outep = usb.util.find_descriptor(
    intf,
    # First OUT endpoint
    custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

inep = usb.util.find_descriptor(
    intf,
    # First IN endpoint
    custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_IN)

assert inep is not None
assert outep is not None

# Load bitstream to send

bitstream_path = sys.argv[1]
bitstream_file = open(bitstream_path, 'rb')
if bitstream_file is None:
    raise ValueError("Failed to open bitstream file: {:s}".format(bitstream_path))

bitstream = bitstream_file.read()
bitstream_file.close()

# Send bitstream over USB

ctrl_request_type = 0x40

ctrl_request_prepare = 0x01
ctrl_request_finalize = 0x02

dev.ctrl_transfer(ctrl_request_type, ctrl_request_prepare, 0, 0)
outep.write(bitstream)
dev.ctrl_transfer(ctrl_request_type, ctrl_request_finalize, 0, 0)
