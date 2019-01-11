/*
 * Copyright (c) 2018, Kentaro Sekimoto
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  -Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *  -Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef COMMON_H_
#define COMMON_H_

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define SUCCESS  0
#define ERROR    -1

#ifndef NULL
#define NULL 0
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "iodefine.h"
#include "rx65n_adc.h"
#include "rx65n_config.h"
#include "rx65n_dac.h"
#include "rx65n_exti.h"
#include "rx65n_ether.h"
#include "rx65n_flash.h"
#include "rx65n_gpio.h"
#include "rx65n_i2c.h"
#include "rx65n_init.h"
#include "rx65n_int.h"
#include "rx65n_pwm.h"
#include "rx65n_rtc.h"
#include "rx65n_sci.h"
#include "rx65n_spi.h"
#include "rx65n_servo.h"
#include "rx65n_timer.h"
#include "rx65n_tpu.h"
#include "rx65n_utils.h"
#ifdef USE_DBG_PRINT
#include "debug_printf.h"
#endif

#endif /* COMMON_H_ */
