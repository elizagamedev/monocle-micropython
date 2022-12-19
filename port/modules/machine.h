/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Damien P. George
 * Copyright (c) 2016 Glenn Ruben Bakke
 * Copyright (c) 2022 Brilliant Labs Inc.
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

#ifndef __MICROPY_INCLUDED_NRF5_MODMACHINE_H__
#define __MICROPY_INCLUDED_NRF5_MODMACHINE_H__

#include "py/mpstate.h"
#include "py/nlr.h"
#include "py/obj.h"

extern const mp_obj_type_t machine_touch_type;
extern const mp_obj_type_t machine_timer_type;
extern const mp_obj_type_t machine_rtcounter_type;
extern const mp_obj_type_t machine_power_type;
extern const mp_obj_type_t machine_fpga_type;
extern const mp_obj_type_t machine_battery_type;

void rtc_init0(void);
void timer_init0(void);
void machine_init(void);

MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(machine_info_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_reset_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_lightsleep_obj);
MP_DECLARE_CONST_FUN_OBJ_0(machine_deepsleep_obj);
NORETURN mp_obj_t machine_bootloader(size_t n_args, const mp_obj_t *args);

#endif // __MICROPY_INCLUDED_NRF5_MODMACHINE_H__
