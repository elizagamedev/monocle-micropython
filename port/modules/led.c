/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Damien P. George
 * Copyright (c) 2015 - 2018 Glenn Ruben Bakke
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

#include "py/runtime.h"

#include "mphalport.h"
#include "mpconfigport.h"

#include "nrfx_twi.h"

#include "driver/max77654.h"

enum {
    LED_RED,
    LED_GREEN
};

STATIC mp_obj_t mod_led___init__(void)
{
    max77654_init();
    max77654_rail_vled(true);

    max77654_led_red(false);
    max77654_led_green(false);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_led___init___obj, mod_led___init__);

static mp_obj_t led_on(mp_obj_t led_in) {
    switch (MP_OBJ_SMALL_INT_VALUE(led_in)) {
        case LED_RED:
            max77654_led_red(true);
            break;
        case LED_GREEN:
            max77654_led_green(true);
            break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_on_obj, led_on);

static mp_obj_t led_off(mp_obj_t led_in) {
    switch (MP_OBJ_SMALL_INT_VALUE(led_in)) {
        case LED_RED:
            max77654_led_red(false);
            break;
        case LED_GREEN:
            max77654_led_green(false);
            break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_off_obj, led_off);

STATIC const mp_rom_map_elem_t led_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_led) },
    { MP_ROM_QSTR(MP_QSTR___init__),    MP_ROM_PTR(&mod_led___init___obj) },

    // methods
    { MP_ROM_QSTR(MP_QSTR_on),          MP_ROM_PTR(&led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_off),         MP_ROM_PTR(&led_off_obj) },

    // constants
    { MP_ROM_QSTR(MP_QSTR_RED),         MP_OBJ_NEW_SMALL_INT(LED_RED) },
    { MP_ROM_QSTR(MP_QSTR_GREEN),       MP_OBJ_NEW_SMALL_INT(LED_GREEN) },
};
STATIC MP_DEFINE_CONST_DICT(led_module_globals, led_module_globals_table);

const mp_obj_module_t led_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&led_module_globals,
};
MP_REGISTER_MODULE(MP_QSTR_led, led_module);
