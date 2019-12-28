/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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
#include "py/mperrno.h"
#include "extmod/machine_pulse.h"

#if MICROPY_PY_MACHINE_PULSE

MP_WEAK mp_uint_t machine_time_pulse_us(mp_hal_pin_obj_t pin, int pulse_level, mp_uint_t timeout_us) {
    mp_uint_t start = mp_hal_ticks_us();
    while (mp_hal_pin_read(pin) != pulse_level) {
        if ((mp_uint_t)(mp_hal_ticks_us() - start) >= timeout_us) {
            return (mp_uint_t)-2;
        }
    }
    start = mp_hal_ticks_us();
    while (mp_hal_pin_read(pin) == pulse_level) {
        if ((mp_uint_t)(mp_hal_ticks_us() - start) >= timeout_us) {
            return (mp_uint_t)-1;
        }
    }
    return mp_hal_ticks_us() - start;
}

STATIC mp_obj_t machine_time_pulse_us_(size_t n_args, const mp_obj_t *args) {
    mp_hal_pin_obj_t pin = mp_hal_get_pin_obj(args[0]);
    int level = 0;
    if (mp_obj_is_true(args[1])) {
        level = 1;
    }
    mp_uint_t timeout_us = 1000000;
    if (n_args > 2) {
        timeout_us = mp_obj_get_int(args[2]);
    }
    mp_uint_t us = machine_time_pulse_us(pin, level, timeout_us);
    // May return -1 or -2 in case of timeout
    return mp_obj_new_int(us);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_time_pulse_us_obj, 2, 3, machine_time_pulse_us_);

MP_WEAK mp_uint_t machine_time_pulse2_us(mp_hal_pin_obj_t pin1, int pulse_level1, mp_hal_pin_obj_t pin2, int pulse_level2, mp_uint_t timeout_us) {
    mp_uint_t start = mp_hal_ticks_us();
    int val1;
    // We wait until one of the pins transitions to the requested state
    while (((val1 = mp_hal_pin_read(pin1)) != pulse_level1) &&
	   (mp_hal_pin_read(pin2) != pulse_level2)) {
        if ((mp_uint_t)(mp_hal_ticks_us() - start) >= timeout_us) {
            mp_raise_OSError(MP_ETIMEDOUT);
        }
    }
    start = mp_hal_ticks_us();
    if (val1 == pulse_level1) {
	// pin1 was first, wait for pin2 to follow
	while (mp_hal_pin_read(pin2) != pulse_level2) {
	    if ((mp_uint_t)(mp_hal_ticks_us() - start) >= timeout_us) {
		mp_raise_OSError(MP_ETIMEDOUT);
	    }
	}
	// Return the difference as positive microseconds
	return mp_hal_ticks_us() - start;
    } else {
	// pin2 was first, wait for pin1 to follow
	while (mp_hal_pin_read(pin1) != pulse_level1) {
	    if ((mp_uint_t)(mp_hal_ticks_us() - start) >= timeout_us) {
		mp_raise_OSError(MP_ETIMEDOUT);
	    }
	}
	// Return the difference as negative microseconds
	return -(int)(mp_hal_ticks_us() - start);
    }
}

STATIC mp_obj_t machine_time_pulse2_us_(size_t n_args, const mp_obj_t *args) {
    mp_hal_pin_obj_t pin1 = mp_hal_get_pin_obj(args[0]);
    mp_hal_pin_obj_t pin2 = mp_hal_get_pin_obj(args[2]);
    int level1 = 0;
    int level2 = 0;
    if (mp_obj_is_true(args[1])) {
        level1 = 1;
    }
    if (mp_obj_is_true(args[3])) {
        level2 = 1;
    }
    mp_uint_t timeout_us = 1000000;
    if (n_args > 4) {
        timeout_us = mp_obj_get_int(args[4]);
    }
    mp_uint_t us = machine_time_pulse2_us(pin1, level1, pin2, level2, timeout_us);
    // May return -1 or -2 in case of timeout and -3 if pin2 is already in the
    // state we should wait for then pin1 changes state.
    return mp_obj_new_int(us);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_time_pulse2_us_obj, 4, 5, machine_time_pulse2_us_);

#endif
