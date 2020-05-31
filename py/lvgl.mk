ifeq ($(LVGL_ENABLE),1)
LIB_LVGL = libs/liblvgl.a

#LittlevGL
LVGL_BINDING_DIR = $(TOP)/lib/lv_bindings
LVGL_DIR = $(LVGL_BINDING_DIR)/lvgl
LVGL_GENERIC_DRV_DIR = $(LVGL_BINDING_DIR)/driver/generic
LV_CFLAGS += -DLV_CONF_INCLUDE_SIMPLE
INC += -I$(LVGL_BINDING_DIR)
#ALL_LVGL_SRC = $(shell find $(LVGL_DIR) -type f -name '*.h') $(LVGL_BINDING_DIR)/lv_conf.h
ALL_LVGL_SRC = $(shell find $(LVGL_DIR) -type f -name '*.h') lvgl/lv_conf.h
LVGL_PP = $(BUILD)/lvgl/lvgl.pp.c
LVGL_MPY = $(BUILD)/lvgl/lv_mpy.c
LVGL_MPY_METADATA = $(BUILD)/lvgl/lv_mpy.json
#QSTR_GLOBAL_DEPENDENCIES += $(LVGL_MPY)
CFLAGS_LVGL += $(LV_CFLAGS) 

$(LVGL_MPY): $(ALL_LVGL_SRC) $(LVGL_BINDING_DIR)/gen/gen_mpy.py 
	$(ECHO) "LVGL-GEN $@"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CPP) $(LV_CFLAGS) -I $(LVGL_BINDING_DIR)/pycparser/utils/fake_libc_include $(INC) $(LVGL_DIR)/lvgl.h > $(LVGL_PP)
	$(Q)$(PYTHON) $(LVGL_BINDING_DIR)/gen/gen_mpy.py -M lvgl -MP lv -MD $(LVGL_MPY_METADATA) -E $(LVGL_PP) $(LVGL_DIR)/lvgl.h > $@

CFLAGS_LVGL += -Wno-unused-function
SRC_LVGL_C += $(subst $(TOP)/,,$(shell find $(LVGL_DIR)/src -type f -name "*.c"))
SRC_LVGL_MPY_C += $(subst $(TOP)/,,$(shell find $(LVGL_GENERIC_DRV_DIR) -type f -name "*.c") $(LVGL_MPY))

#lodepng
LODEPNG_DIR = $(TOP)/lib/lv_bindings/driver/png/lodepng
MP_LODEPNG_C = $(TOP)/lib/lv_bindings/driver/png/mp_lodepng.c
ALL_LODEPNG_SRC = $(shell find $(LODEPNG_DIR) -type f)
LODEPNG_MODULE = $(BUILD)/lodepng/mp_lodepng.c
LODEPNG_C = $(BUILD)/lodepng/lodepng.c
LODEPNG_PP = $(BUILD)/lodepng/lodepng.pp.c
INC += -I$(LODEPNG_DIR)
LODEPNG_CFLAGS += -DLODEPNG_NO_COMPILE_ENCODER -DLODEPNG_NO_COMPILE_DISK -DLODEPNG_NO_COMPILE_ALLOCATORS
CFLAGS_LVGL += $(LODEPNG_CFLAGS)

$(LODEPNG_MODULE): $(ALL_LODEPNG_SRC) $(LVGL_BINDING_DIR)/gen/gen_mpy.py 
	$(ECHO) "LODEPNG-GEN $@"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CPP) $(LODEPNG_CFLAGS) $(INC) -I $(LVGL_BINDING_DIR)/pycparser/utils/fake_libc_include $(LODEPNG_DIR)/lodepng.h > $(LODEPNG_PP)
	$(Q)$(PYTHON) $(LVGL_BINDING_DIR)/gen/gen_mpy.py -M lodepng -E $(LODEPNG_PP) $(LODEPNG_DIR)/lodepng.h > $@

$(LODEPNG_C): $(LODEPNG_DIR)/lodepng.cpp $(LODEPNG_DIR)/*
	$(Q)mkdir -p $(dir $@)
	cp $< $@

SRC_LVGL_C += $(subst $(TOP)/,,$(LODEPNG_C))
SRC_LVGL_MPY_C += $(subst $(TOP)/,,$(MP_LODEPNG_C) $(LODEPNG_MODULE))
QSTR_GLOBAL_DEPENDENCIES += $(SRC_LVGL_MPY_C)

CFLAGS += $(CFLAGS_LVGL)

OBJ_LVGL_C += $(addprefix $(BUILD)/, $(SRC_LVGL_C:.c=.o) $(SRC_LVGL_MPY_C:.c=.o))
OBJ_LVGL += $(OBJ_LVGL_C) 

AR_LVGL_FLAGS = rcs

$(LIB_LVGL): $(OBJ_LVGL)
	$(ECHO) "LIB $@"
	$(MKDIR) -p libs
	$(Q)$(AR) $(AR_LVGL_FLAGS) $(LIB_LVGL) $^

endif