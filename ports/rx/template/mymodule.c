/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Kentaro Sekimoto
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

#include "mymodule.h"
#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "portmodules.h"
#include <string.h>
#include <stdio.h>

#if MICROPY_PY_MYMODULE
// ===== micropyhton class in module definition
const mp_obj_type_t mymodule_helloObj_type;

// this is the actual C-structure for our new object
typedef struct _mymodule_hello_obj_t {
    // base represents some basic information, like type
    mp_obj_base_t base;
    // a member created by us
    uint8_t hello_number;
} mymodule_hello_obj_t;

mp_obj_t mymodule_hello_make_new( const mp_obj_type_t *type,
                                  size_t n_args,
                                  size_t n_kw,
                                  const mp_obj_t *args ) {
    // this checks the number of arguments (min 1, max 1);
    // on error -> raise python exception
    mp_arg_check_num(n_args, n_kw, 1, 1, true);
    // create a new object of our C-struct type
    mymodule_hello_obj_t *self = m_new_obj(mymodule_hello_obj_t);
    // give it a type
    self->base.type = &mymodule_helloObj_type;
    // set the member number with the first argument of the constructor
    self->hello_number = mp_obj_get_int(args[0]);
    return MP_OBJ_FROM_PTR(self);
}

STATIC void mymodule_hello_print( const mp_print_t *print,
                                  mp_obj_t self_in,
                                  mp_print_kind_t kind ) {
    // get a ptr to the C-struct of the object
    mymodule_hello_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // print the number
    printf ("Hello(%u)", self->hello_number);
}

STATIC mp_obj_t mymodule_hello_increment(mp_obj_t self_in) {
    mymodule_hello_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->hello_number += 1;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mymodule_hello_increment_obj, mymodule_hello_increment);

// creating the table of global members
STATIC const mp_rom_map_elem_t mymodule_hello_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_inc), MP_ROM_PTR(&mymodule_hello_increment_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mymodule_hello_locals_dict,
                            mymodule_hello_locals_dict_table);

// create the class-object itself
const mp_obj_type_t mymodule_helloObj_type = {
    // "inherit" the type "type"
    { &mp_type_type },
     // give it a name
    .name = MP_QSTR_helloObj,
     // give it a print-function
    .print = mymodule_hello_print,
     // give it a constructor
    .make_new = mymodule_hello_make_new,
     // and the global members
    .locals_dict = (mp_obj_dict_t*)&mymodule_hello_locals_dict,
};

// ===== micropyhton function in module definition

STATIC mp_obj_t mymodule_hello(void) {
    printf("Hello world!\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mymodule_hello_obj, mymodule_hello);

// ===== micropyhton module definition

STATIC const mp_map_elem_t mymodule_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_mymodule) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_hello), (mp_obj_t)&mymodule_hello_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_helloObj), (mp_obj_t)&mymodule_helloObj_type },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_mymodule_globals, mymodule_globals_table);

const mp_obj_module_t mp_module_mymodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_mymodule_globals,
};
#endif
