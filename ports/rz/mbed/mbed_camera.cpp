/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Kentaro Sekimoto
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

#include "mbed.h"
#include "rtos.h"
#include "dcache-control.h"
#include "MT9V111_config.h"
#include "OV2640_config.h"
#include "OV2640_regs.h"
#include "OV5642_config.h"
#include "OV7725_config.h"
#include "OV7670_config.h"
#include "RaspberryPi_config.h"
#include "RaspberryPi_832x480_config.h"
#include "RaspberryPi_wide_angle_config.h"
#ifdef MBED_CONF_APP_CAMERA
#undef MBED_CONF_APP_CAMERA
#endif
#define MBED_CONF_APP_CAMERA    1
#include "EasyAttach_CameraAndLCD.h"
#include "sccb.h"
#include "rz_buf.h"
#include "display.h"
#include "mbed_lcd.h"
#include "sccb.h"
#include "mbed_camera.h"

extern DisplayBase g_mbed_display;

#if defined(TARGET_RZ_A1H)
static const PinName cmos_camera_pin[11] = {
    /* data pin */
    P2_7, P2_6, P2_5, P2_4, P2_3, P2_2, P2_1, P2_0,
    /* control pin */
    P10_0, /* DV0_CLK   */
    P1_0, /* DV0_Vsync */
    P1_1 /* DV0_Hsync */
};
#endif
#if defined(TARGET_GR_LYCHEE)
static const PinName cmos_camera_pin[11] = {
    /* data pin */
    P1_0, P1_1, P1_2, P1_3, P1_8, P1_9, P1_10, P1_11,
    /* control pin */
    P7_8, /* DV0_CLK   */
    P7_9, /* DV0_Vsync */
    P7_10 /* DV0_Hsync */
};
#endif
#if defined(TARGET_GR_MANGO)
static const PinName cmos_camera_pin[11] = {
    /* data pin */
    P7_4, P7_5, P9_7, P9_6, P9_5, P9_4, P9_3, P9_2,
    /* control pin */
    P7_0, /* VIO_CLK   */
    P7_1, /* VIO_VD */
    P7_3 /* VIO_HD */
};
#elif defined(TARGET_RZ_A2XX)
static const PinName cmos_camera_pin[11] = {
    /* data pin */
    PE_1, PE_2, PE_3, PE_4, PE_5, PE_6, PH_0, PH_1,
    /* control pin */
    P6_1, /* VIO_CLK   */
    P6_2, /* VIO_VD */
    P6_3 /* VIO_HD */
};
#endif

static bool mbed_camera_init_camera_id(DisplayBase& Display, uint16_t cap_width, uint16_t cap_height, uint16_t camera_id, uint16_t module, uint16_t format, uint32_t reset_level) {
    DisplayBase::graphics_error_t error;
    if (camera_id == CAMERA_CVBS) {
        error = Display.Graphics_Video_init(DisplayBase::INPUT_SEL_VDEC, NULL);
        if (error != DisplayBase::GRAPHICS_OK) {
            return false;
        } else {
            return true;
        }
    }

    DisplayBase::video_input_sel_t video_input_sel;
    if (module == MODULE_MIPI) {
        // do nothing
    } else {
        #if defined(TARGET_RZ_A1H)
        #if MBED_CONF_APP_SHIELD_TYPE == SHIELD_AUDIO_CAMERA
        DigitalOut pwdn(P3_12);
        pwdn = 0;
        ThisThread::sleep_for(1ms + 1ms);
        #elif MBED_CONF_APP_SHIELD_TYPE == SHIELD_WIRELESS_CAMERA
        DigitalOut pwdn(P3_15);
        DigitalOut rstb(P3_14);
        pwdn = 0;
        rstb = 0;
        ThisThread::sleep_for(10ms + 1ms);
        rstb = 1;
        ThisThread::sleep_for(1ms + 1ms);
        #endif
        #endif
        #if defined(TARGET_GR_LYCHEE)
        DigitalOut pwdn(P7_11);
        DigitalOut rstb(P2_3);
        pwdn = 0;
        rstb = 0;
        ThisThread::sleep_for(10ms + 1ms);
        rstb = 1;
        ThisThread::sleep_for(1ms + 1ms);
        #endif
        #if defined(TARGET_GR_MANGO)
        DigitalOut pwdn(P1_3);
        DigitalOut rstb(P1_4);
        pwdn = 0;
        if (reset_level == 0) {
            // reset active low
            rstb = 0;
            // ThisThread::sleep_for(10ms + 1ms);
            ThisThread::sleep_for(300ms);
            rstb = 1;
        } else {
            // reset active high
            rstb = 1;
            ThisThread::sleep_for(10ms + 1ms);
            rstb = 0;
        }
        // ThisThread::sleep_for(1ms + 1ms);
        ThisThread::sleep_for(300ms);
        #elif defined(TARGET_RZ_A2XX)
        DigitalOut camera_stby(PE_0);
        camera_stby = 0;
        ThisThread::sleep_for(1ms + 1ms);
        #endif
    }

    /* camera input port setting */
    if (module == MODULE_VDC) {
        video_input_sel = DisplayBase::INPUT_SEL_EXT;
        error = Display.Graphics_Dvinput_Port_Init((PinName *)cmos_camera_pin, 11);
    } else if (module == MODULE_CEU) {
        video_input_sel = DisplayBase::INPUT_SEL_CEU;
        error = Display.Graphics_Ceu_Port_Init((PinName *)cmos_camera_pin, 11);
    } else {
        video_input_sel = DisplayBase::INPUT_SEL_MIPI;
        error = DisplayBase::GRAPHICS_OK;
    }
    if (error != DisplayBase::GRAPHICS_OK) {
        return false;
    }

    if (module == MODULE_MIPI) {
        DisplayBase::video_mipi_param_t mipi_config;
        DisplayBase::video_vin_setup_t vin_setup;
        switch (camera_id) {
            case CAMERA_RAS_PI_DEF: {
                RaspberryPi_config RaspberryPi_camera_cfg;
                RaspberryPi_camera_cfg.Initialise();
                error = Display.Graphics_Video_init(video_input_sel, &mipi_config, &vin_setup);
                break;
            }
            case CAMERA_RAS_PI_WIDE_ANGLE: {
                RaspberryPi_wide_angle_config RaspberryPi_wide_angle_camera_cfg;
                RaspberryPi_wide_angle_camera_cfg.Initialise();
                RaspberryPi_wide_angle_camera_cfg.SetMipiConfig(&mipi_config);
                RaspberryPi_wide_angle_camera_cfg.SetVinSetup(&vin_setup);
                error = Display.Graphics_Video_init(video_input_sel, &mipi_config, &vin_setup);
                break;
            }
            case CAMERA_RAS_PI_832X480: {
                RaspberryPi_832x480_config RaspberryPi_832x480_camera_cfg;
                RaspberryPi_832x480_camera_cfg.Initialise();
                RaspberryPi_832x480_camera_cfg.SetMipiConfig(&mipi_config);
                RaspberryPi_832x480_camera_cfg.SetVinSetup(&vin_setup);
                error = Display.Graphics_Video_init(video_input_sel, &mipi_config, &vin_setup);
                break;
            }
            default:
                error = DisplayBase::GRAPHICS_PARAM_RANGE_ERR;
                break;
        }
    } else {
        DisplayBase::video_ext_in_config_t ext_in_config;
        switch (camera_id) {
            case CAMERA_MT9V111: {
                MT9V111_config MT9V111_camera_cfg;
                MT9V111_camera_cfg.Initialise();
                MT9V111_camera_cfg.SetExtInConfig(&ext_in_config);
                if (cap_width != 0) {
                    ext_in_config.cap_width = cap_width;
                }
                if (cap_height != 0) {
                    ext_in_config.cap_height = cap_height;
                }
                error = Display.Graphics_Video_init(video_input_sel, &ext_in_config);
                break;
            }
            case CAMERA_OV7725: {
                OV7725_config OV7725_camera_cfg;
                OV7725_camera_cfg.Initialise();
                OV7725_camera_cfg.SetExtInConfig(&ext_in_config);
                if (cap_width != 0) {
                    ext_in_config.cap_width = cap_width;
                }
                if (cap_height != 0) {
                    ext_in_config.cap_height = cap_height;
                }
                error = Display.Graphics_Video_init(video_input_sel, &ext_in_config);
                break;
            }
            case CAMERA_OV5642: {
                OV5642_config OV5642_camera_cfg;
                OV5642_camera_cfg.Initialise();
                OV5642_camera_cfg.SetExtInConfig(&ext_in_config);
                if (cap_width != 0) {
                    ext_in_config.cap_width = cap_width;
                }
                if (cap_height != 0) {
                    ext_in_config.cap_height = cap_height;
                }
                error = Display.Graphics_Video_init(video_input_sel, &ext_in_config);
                break;
            }
            case CAMERA_OV2640: {
                OV2640_config OV2640_camera_cfg;
                OV2640_camera_cfg.Initialise();
                OV2640_camera_cfg.SetExtInConfig(&ext_in_config);
                if (cap_width != 0) {
                    ext_in_config.cap_width = cap_width;
                }
                if (cap_height != 0) {
                    ext_in_config.cap_height = cap_height;
                }
                error = Display.Graphics_Video_init(video_input_sel, &ext_in_config);
                break;
            }
            case CAMERA_OV7670: {
                OV7670_config OV7670_camera_cfg;
                // OV7670_camera_cfg.Initialise();
                OV7670_camera_cfg.Initialise((DisplayBase::video_extin_format_t)format);
                OV7670_camera_cfg.SetExtInConfig(&ext_in_config);
                // OV7670_camera_cfg.SetExtInConfig(&ext_in_config, (DisplayBase::video_extin_format_t)format);
                if (cap_width != 0) {
                    ext_in_config.cap_width = cap_width;
                }
                if (cap_height != 0) {
                    ext_in_config.cap_height = cap_height;
                }
                error = Display.Graphics_Video_init(video_input_sel, &ext_in_config);
                break;
            }
            default:
                error = DisplayBase::GRAPHICS_PARAM_RANGE_ERR;
                break;
        }
    }
    if (error != DisplayBase::GRAPHICS_OK) {
        return false;
    } else {
        return true;
    }
}

#if 0
static void IntCallbackFunc_Vfield(DisplayBase::int_type_t int_type) {
    // need to add interrupt process
}
#endif

bool mbed_start_video_camera(camera_t *camera) {
    DisplayBase::video_format_t format = (DisplayBase::video_format_t)camera->vformat;
    DisplayBase::wr_rd_swa_t swa = (DisplayBase::wr_rd_swa_t)camera->swa;
    DisplayBase::video_input_channel_t ch = (DisplayBase::video_input_channel_t)camera->input_ch;
    uint32_t size = camera->stride * camera->vw;
    for (uint32_t i = 0; i < size; i += 2) {
        camera->buf[i + 0] = 0x88;
        camera->buf[i + 1] = 0x00;
    }
    dcache_clean(camera->buf, (size_t)size);
    // mbed_display->Graphics_Irq_Handler_Set(DisplayBase::INT_TYPE_S0_VFIELD, 0, IntCallbackFunc_Vfield);
    DisplayBase::graphics_error_t error;
    error = g_mbed_display.Video_Write_Setting(
        ch,
        DisplayBase::COL_SYS_NTSC_358,
        (void *)camera->buf,
        (unsigned int)camera->stride,
        format,
        swa,
        (unsigned short)camera->vw,
        (unsigned short)camera->hw
        );
    ThisThread::sleep_for(50ms);

    /* Video write process start */
    error = g_mbed_display.Video_Start(ch);
    #if 0
    if (error == DisplayBase::GRAPHICS_OK) {
        if (camera->module == MODULE_VDC) {
            /* Video write process stop */
            error = g_mbed_display.Video_Stop(ch);
            if (error == DisplayBase::GRAPHICS_OK) {
                /* Video write process start */
                error = g_mbed_display.Video_Start(ch);
            }
        }
    }
    #endif
    if (error == DisplayBase::GRAPHICS_OK) {
        return false;
    } else {
        return true;
    }
}

void mbed_camera_init_params(camera_t *camera) {
    switch (camera->camera_id) {
        case CAMERA_CVBS:
            camera->hw = DEF_CAMERA_WIDTH;
            camera->vw = DEF_CAMERA_HEIGHT;
            camera->module = MODULE_VDC;
            break;
        case CAMERA_MT9V111:
            camera->hw = 640;
            camera->vw = 468;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
        case CAMERA_OV7725:
            camera->hw = 640;
            camera->vw = 480;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
        case CAMERA_OV5642:
            camera->hw = 640;
            camera->vw = 480;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
        case CAMERA_OV2640:
            camera->hw = 640;
            camera->vw = 480;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
        case CAMERA_OV7670:
            camera->hw = 640;
            camera->vw = 480;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
        case CAMERA_WIRELESS_CAMERA:
            camera->hw = 640;
            camera->vw = 480;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
        case CAMERA_RASPBERRY_PI:
            camera->hw = 1280;
            camera->vw = 720;
            camera->module = MODULE_MIPI;
            break;
        case CAMERA_RASPBERRY_PI_WIDE_ANGLE:
            camera->hw = 640;
            camera->vw = 416;
            camera->module = MODULE_MIPI;
            break;
        case CAMERA_RASPBERRY_PI_832X480:
            camera->hw = 832;
            camera->vw = 480;
            camera->module = MODULE_MIPI;
            break;
        default:
            camera->hw = DEF_CAMERA_WIDTH;
            camera->vw = DEF_CAMERA_HEIGHT;
            #if defined(TARGET_RZ_A2M_EVB) || defined(TARGET_RZ_A2M_EVB_HF) || defined(TARGET_RZ_A2M_SBEV) || defined(TARGET_SEMB1402)
            camera->module = MODULE_CEU;
            #else
            camera->module = MODULE_VDC;
            #endif
            break;
    }
    switch (camera->vformat) {
        case VFORMAT_YCBCR422:
            camera->depth = 2;
            break;
        case VFORMAT_RGB565:
            camera->depth = 2;
            break;
        case VFORMAT_RGB888:
            camera->depth = 3;
            break;
        case VFORMAT_RAW8:
            camera->depth = 1;
            break;
        default:
            camera->depth = 2;
            break;
    }
    if (camera->stride == 0) {
        camera->stride = ((camera->hw * camera->depth + 31u) & ~31u);
    }
}

bool mbed_camera_init(camera_t *camera) {
    mbed_camera_init_params(camera);
    if (camera->buf == (uint8_t *)NULL) {
        camera->buf = (uint8_t *)rz_malloc((size_t)(camera->stride * camera->vw) + CAMERA_BUF_ALIGN);
        if (camera->buf == (uint8_t *)NULL) {
            return false;
        }
        camera->buf = (uint8_t *)(((uint32_t)camera->buf + CAMERA_BUF_ALIGN) & ~(CAMERA_BUF_ALIGN - 1));
    }
    return mbed_camera_init_camera_id(g_mbed_display, (uint16_t)camera->hw, (uint16_t)camera->vw, (uint16_t)camera->camera_id, (uint16_t)camera->module, camera->cformat, camera->reset_level);
}

static void camera_reset_hardware(uint32_t reset_level) {
    // hardware reset
    DigitalOut pwdn(P1_3);
    DigitalOut rstb(P1_4);
    pwdn = 0;
    if (reset_level == 0) {
        rstb = 0;
        ThisThread::sleep_for(10ms + 1ms);
        rstb = 1;
    } else {
        rstb = 1;
        ThisThread::sleep_for(10ms + 1ms);
        rstb = 0;
    }
    ThisThread::sleep_for(1ms + 1ms);
}

bool camera_type1_reg_tbl_write(uint8_t addr, const uint8_t *tbl, size_t size) {
    int ret = sccb_reg_write_n(addr, tbl, size);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool camera_type1_reg_write(uint8_t addr, uint8_t reg, uint8_t v) {
    int ret = sccb_reg_write(addr, reg, v);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool camera_type1_reg_read(uint8_t addr, uint8_t reg, uint8_t *v) {
    int ret = sccb_reg_read(addr, reg, v);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool camera_type2_reg_tbl_write(uint8_t addr, const uint8_t *tbl, size_t size) {
    int ret = sccb2_reg_write_n(addr, tbl, size);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool camera_type2_reg_write(uint8_t addr, uint16_t reg, uint8_t v) {
    int ret = sccb2_reg_write(addr, (uint8_t)(reg >> 8), (uint8_t)(reg & 0xff), v);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

bool camera_type2_reg_read(uint8_t addr, uint16_t reg, uint8_t *v) {
    int ret = sccb2_reg_read(addr, (uint8_t)(reg >> 8), (uint8_t)(reg & 0xff), v);
    if (ret == 0) {
        return true;
    } else {
        return false;
    }
}

static bool camera_type1_reset(uint8_t addr) {
    // software reset command
    const char table_sel_cmd[2] = {0xff, 0x01};
    bool flag = camera_type1_reg_tbl_write(addr, (const uint8_t *)&table_sel_cmd, 2);
    if (!flag) {
        return false;
    }
    ThisThread::sleep_for(1ms);
    const char sw_reset_cmd[2] = {0x12, 0x80};
    flag = camera_type1_reg_tbl_write(addr, (const uint8_t *)&sw_reset_cmd, 2);
    ThisThread::sleep_for(1ms);
    if (!flag) {
        return false;
    }
    return true;
}

static uint16_t camera_type1_get_product_id(uint8_t addr, uint32_t reset_level) {
    camera_reset_hardware(reset_level);
    camera_type1_reset(addr);
    int ret = 0;
    uint16_t id = 0;
    uint8_t cmd;
    uint8_t v;
    cmd = 0x0a;
    I2C mI2c_(I2C_SDA, I2C_SCL);
    mI2c_.frequency(150000);
    ret = mI2c_.write((int)addr, (const char *)&cmd, 1);
    if (ret != 0) {
        return id;
    }
    ret = mI2c_.read((int)(addr + 1), (char *)&v, 1);
    if (ret != 0) {
        return id;
    }
    id = (uint16_t)v;
    cmd = 0x0b;
    ret = mI2c_.write((int)addr, (const char *)&cmd, 1);
    if (ret != 0) {
        return id;
    }
    ret = mI2c_.read((int)(addr + 1), (char *)&v, 1);
    if (ret != 0) {
        return id;
    }
    id = (id << 8) + (uint16_t)v;
    return id;
}

static uint16_t camera_type2_get_product_id(uint8_t addr, uint32_t reset_level) {
    camera_reset_hardware(reset_level);
    int ret = 0;
    uint16_t id = 0;
    uint8_t cmd[2];
    uint8_t v;
    cmd[0] = 0x30;
    cmd[1] = 0x0a;
    I2C mI2c_(I2C_SDA, I2C_SCL);
    mI2c_.frequency(150000);
    ret = mI2c_.write((int)addr, (const char *)&cmd[0], 2);
    if (ret != 0) {
        return id;
    }
    ret = mI2c_.read((int)(addr + 1), (char *)&v, 1);
    if (ret != 0) {
        return id;
    }
    id = (uint16_t)v;
    cmd[0] = 0x30;
    cmd[1] = 0x0b;
    ret = mI2c_.write((int)addr, (const char *)&cmd[0], 2);
    if (ret != 0) {
        return id;
    }
    ret = mI2c_.read((int)(addr + 1), (char *)&v, 1);
    if (ret != 0) {
        return id;
    }
    id = (id << 8) + (uint16_t)v;
    return id;
}

uint16_t camera_get_ov_product_id(uint8_t addr, uint32_t reset_level) {
    switch (addr) {
        case 0x78:
            return camera_type2_get_product_id(addr, reset_level);
        case 0x42:
        default:
            return camera_type1_get_product_id(addr, reset_level);
    }
}
