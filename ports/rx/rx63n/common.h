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

#include "stdint.h"
#include "stdbool.h"
#include "iodefine.h"
#include "rx63n_adc.h"
#include "rx63n_config.h"
#include "rx63n_dac.h"
#include "rx63n_ether.h"
#include "rx63n_exti.h"
#include "rx63n_flash.h"
#include "rx63n_gpio.h"
#include "rx63n_i2c.h"
#include "rx63n_init.h"
#include "rx63n_int.h"
#include "rx63n_pwm.h"
#include "rx63n_rtc.h"
#include "rx63n_sci.h"
#include "rx63n_spi.h"
#include "rx63n_servo.h"
#include "rx63n_timer.h"
#include "rx63n_tpu.h"
#include "rx63n_utils.h"
#ifdef USE_DBG_PRINT
#include "debug_printf.h"
#endif

#endif /* COMMON_H_ */
