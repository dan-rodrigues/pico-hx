// fpga_spi.h
//
// Copyright (C) 2021 Dan Rodrigues <danrr.gh.oss@gmail.com>
//
// SPDX-License-Identifier: MIT

#include "pico/stdlib.h"

void __not_in_flash_func(fpga_upload_bitstream)(const uint8_t *bitstream, size_t len);
