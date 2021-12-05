#ifndef PORTS_RZ_LV_LV_MPY_H_
#define PORTS_RZ_LV_LV_MPY_H_

#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include "lib/lv_bindings/lvgl/lvgl.h"

typedef struct _spi_device_handle_t {
    uint32_t dummy;
} spi_device_handle_t;

//////////////////////////////////////////////////////////////////////////////
// A read-only buffer that contains a C pointer
// Used to communicate function pointers to lvgl Micropython bindings
//

typedef struct mp_ptr_t
{
    mp_obj_base_t base;
    void *ptr;
} mp_ptr_t;

STATIC mp_int_t mp_ptr_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags)
{
    mp_ptr_t *self = MP_OBJ_TO_PTR(self_in);

    if (flags & MP_BUFFER_WRITE) {
        // read-only ptr
        return 1;
    }

    bufinfo->buf = &self->ptr;
    bufinfo->len = sizeof(self->ptr);
    bufinfo->typecode = BYTEARRAY_TYPECODE;
    return 0;
}

#define PTR_OBJ(ptr_global) ptr_global ## _obj

#define DEFINE_PTR_OBJ_TYPE(ptr_obj_type, ptr_type_qstr)\
STATIC const mp_obj_type_t ptr_obj_type = {\
    { &mp_type_type },\
    .name = ptr_type_qstr,\
    .buffer_p = { .get_buffer = mp_ptr_get_buffer }\
}

#define DEFINE_PTR_OBJ(ptr_global)\
DEFINE_PTR_OBJ_TYPE(ptr_global ## _type, MP_QSTR_ ## ptr_global);\
STATIC const mp_ptr_t PTR_OBJ(ptr_global) = {\
    { &ptr_global ## _type },\
    &ptr_global\
}

#define NEW_PTR_OBJ(name, value)\
({\
    DEFINE_PTR_OBJ_TYPE(ptr_obj_type, MP_QSTR_ ## name);\
    mp_ptr_t *self = m_new_obj(mp_ptr_t);\
    self->base.type = &ptr_obj_type;\
    self->ptr = value;\
    MP_OBJ_FROM_PTR(self);\
})

#endif /* PORTS_RZ_LV_LV_MPY_H_ */
