/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef PORTS_RZ_MBED_MBED_SD_H_
#define PORTS_RZ_MBED_MBED_SD_H_

#ifdef __cplusplus
extern "C" {
#endif

void mbed_sdcard_init(void);
void mbed_sdcard_deinit(void);
bool mbed_sdcard_is_present(void);
bool mbed_sdcard_power_on(void);
void mbed_sdcard_power_off(void);
uint64_t mbed_sdcard_get_capacity_in_bytes(void);
uint mbed_sdcard_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks);
uint mbed_sdcard_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks);

#ifdef __cplusplus
};
#endif

#endif /* PORTS_RZ_MBED_MBED_SD_H_ */
