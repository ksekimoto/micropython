#ifndef PORTS_RZ_MBED_MBED_LCD_H_
#define PORTS_RZ_MBED_MBED_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define SD_7INCH                            (0u)  /*  800x480@60 */
#define SVGA                                (1u)  /*  800x600@60 */
#define XGA                                 (2u)  /* 1024x768@60 */
#define HD_720p                             (3u)  /* 1280x720@60 */

#ifndef LCD_SIZE
#define LCD_SIZE                            HD_720p /* Select SD_7INCH, SVGA, XGA, or HD_720p */
#endif

/* LCD Parameter */

// LCD shield
#define SL_SVGA_LCD_INPUT_CLOCK                     (66.67)  /* not use */
#define SL_SVGA_LCD_OUTPUT_CLOCK                    (40.0003)
#define SL_SVGA_LCD_PIXEL_WIDTH                     (800u)
#define SL_SVGA_LCD_PIXEL_HEIGHT                    (600u)
#define SL_SVGA_LCD_H_BACK_PORCH                    (88u)
#define SL_SVGA_LCD_H_FRONT_PORCH                   (40u)
#define SL_SVGA_LCD_H_SYNC_WIDTH                    (128u)
#define SL_SVGA_LCD_V_BACK_PORCH                    (23u)
#define SL_SVGA_LCD_V_FRONT_PORCH                   (1u)
#define SL_SVGA_LCD_V_SYNC_WIDTH                    (4u)

#define SL_XGA_LCD_INPUT_CLOCK                     (66.67)  /* not use */
#define SL_XGA_LCD_OUTPUT_CLOCK                    (65.0002)
#define SL_XGA_LCD_PIXEL_WIDTH                     (1024u)
#define SL_XGA_LCD_PIXEL_HEIGHT                    (768u)
#define SL_XGA_LCD_H_BACK_PORCH                    (160u)
#define SL_XGA_LCD_H_FRONT_PORCH                   (24u)
#define SL_XGA_LCD_H_SYNC_WIDTH                    (136u)
#define SL_XGA_LCD_V_BACK_PORCH                    (29u)
#define SL_XGA_LCD_V_FRONT_PORCH                   (3u)
#define SL_XGA_LCD_V_SYNC_WIDTH                    (6u)

#define SL_HD_720P_LCD_INPUT_CLOCK                     (66.67)  /* not use */
#define SL_HD_720P_LCD_OUTPUT_CLOCK                    (74.1800)
#define SL_HD_720P_LCD_PIXEL_WIDTH                     (1280u)
#define SL_HD_720P_LCD_PIXEL_HEIGHT                    (720u)
#define SL_HD_720P_LCD_H_BACK_PORCH                    (220u)
#define SL_HD_720P_LCD_H_FRONT_PORCH                   (70u)
#define SL_HD_720P_LCD_H_SYNC_WIDTH                    (80u)
#define SL_HD_720P_LCD_V_BACK_PORCH                    (20u)
#define SL_HD_720P_LCD_V_FRONT_PORCH                   (5u)
#define SL_HD_720P_LCD_V_SYNC_WIDTH                    (5u)

#define TFT_43_LCD_INPUT_CLOCK                     (64.0)  /* not use */
#define TFT_43_LCD_OUTPUT_CLOCK                    (9)
#define TFT_43_LCD_PIXEL_WIDTH                     (480u)
#define TFT_43_LCD_PIXEL_HEIGHT                    (272u)
#define TFT_43_LCD_H_BACK_PORCH                    (43u)
#define TFT_43_LCD_H_FRONT_PORCH                   (8u)
#define TFT_43_LCD_H_SYNC_WIDTH                    (1u)
#define TFT_43_LCD_V_BACK_PORCH                    (12u)
#define TFT_43_LCD_V_FRONT_PORCH                   (4u)
#define TFT_43_LCD_V_SYNC_WIDTH                    (10u)

#define TFT_5_LCD_INPUT_CLOCK                     (66.67)  /* not use */
#define TFT_5_LCD_OUTPUT_CLOCK                    (33.26)
#define TFT_5_LCD_PIXEL_WIDTH                     (800u)
#define TFT_5_LCD_PIXEL_HEIGHT                    (480u)
#define TFT_5_LCD_H_BACK_PORCH                    (128u)
#define TFT_5_LCD_H_FRONT_PORCH                   (92u)
#define TFT_5_LCD_H_SYNC_WIDTH                    (20u)
#define TFT_5_LCD_V_BACK_PORCH                    (35u)
#define TFT_5_LCD_V_FRONT_PORCH                   (5u)
#define TFT_5_LCD_V_SYNC_WIDTH                    (10u)

#define RSK_TFT_LCD_INPUT_CLOCK                     (66.67)  /* not use */
#define RSK_TFT_LCD_OUTPUT_CLOCK                    (33.26)
#define RSK_TFT_LCD_PIXEL_WIDTH                     (800u)
#define RSK_TFT_LCD_PIXEL_HEIGHT                    (480u)
#define RSK_TFT_LCD_H_BACK_PORCH                    (128u)
#define RSK_TFT_LCD_H_FRONT_PORCH                   (92u)
#define RSK_TFT_LCD_H_SYNC_WIDTH                    (20u)
#define RSK_TFT_LCD_V_BACK_PORCH                    (35u)
#define RSK_TFT_LCD_V_FRONT_PORCH                   (5u)
#define RSK_TFT_LCD_V_SYNC_WIDTH                    (10u)

#define SD_7INCH_LCD_INPUT_CLOCK                        (66.67)  /* not use */
#define SD_7INCH_LCD_OUTPUT_CLOCK                       (33.26)
#define SD_7INCH_LCD_PIXEL_WIDTH                        (800u)
#define SD_7INCH_LCD_PIXEL_HEIGHT                       (480u)
#define SD_7INCH_LCD_H_BACK_PORCH                       (128u)
#define SD_7INCH_LCD_H_FRONT_PORCH                      (92u)
#define SD_7INCH_LCD_H_SYNC_WIDTH                       (20u)
#define SD_7INCH_LCD_V_BACK_PORCH                       (35u)
#define SD_7INCH_LCD_V_FRONT_PORCH                      (5u)
#define SD_7INCH_LCD_V_SYNC_WIDTH                       (10u)

#define SVGA_LCD_INPUT_CLOCK                            (66.67)  /* not use */
#define SVGA_LCD_OUTPUT_CLOCK                           (40.0003)
#define SVGA_LCD_PIXEL_WIDTH                            (800u)
#define SVGA_LCD_PIXEL_HEIGHT                           (600u)
#define SVGA_LCD_H_BACK_PORCH                           (88u)
#define SVGA_LCD_H_FRONT_PORCH                          (40u)
#define SVGA_LCD_H_SYNC_WIDTH                           (128u)
#define SVGA_LCD_V_BACK_PORCH                           (20u)
#define SVGA_LCD_V_FRONT_PORCH                          (4u)
#define SVGA_LCD_V_SYNC_WIDTH                           (4u)

#define XGA_LCD_INPUT_CLOCK                             (66.67)  /* not use */
#define XGA_LCD_OUTPUT_CLOCK                            (65.0002)
#define XGA_LCD_PIXEL_WIDTH                             (1024u)
#define XGA_LCD_PIXEL_HEIGHT                            (768u)
#define XGA_LCD_H_BACK_PORCH                            (160u)
#define XGA_LCD_H_FRONT_PORCH                           (24u)
#define XGA_LCD_H_SYNC_WIDTH                            (136u)
#define XGA_LCD_V_BACK_PORCH                            (28u)
#define XGA_LCD_V_FRONT_PORCH                           (4u)
#define XGA_LCD_V_SYNC_WIDTH                            (6u)

#define HD_720P_LCD_INPUT_CLOCK                         (66.67)  /* not use */
#define HD_720P_LCD_OUTPUT_CLOCK                        (74.2500)
#define HD_720P_LCD_PIXEL_WIDTH                         (1280u)
#define HD_720P_LCD_PIXEL_HEIGHT                        (720u)
#define HD_720P_LCD_H_BACK_PORCH                        (216u)
#define HD_720P_LCD_H_FRONT_PORCH                       (72u)
#define HD_720P_LCD_H_SYNC_WIDTH                        (80u)
#define HD_720P_LCD_V_BACK_PORCH                        (22u)
#define HD_720P_LCD_V_FRONT_PORCH                       (3u)
#define HD_720P_LCD_V_SYNC_WIDTH                        (5u)


// lcd type
#if defined(LCD_LVDS)
#undef LCD_LVDS
#endif
#if defined(GR_PEACH_4_3INCH_SHIELD)
#undef GR_PEACH_4_3INCH_SHIELD
#endif
#if defined(GR_PEACH_7_1INCH_SHIELD)
#undef GR_PEACH_7_1INCH_SHIELD
#endif
#if defined(GR_PEACH_DISPLAY_SHIELD)
#undef GR_PEACH_DISPLAY_SHIELD
#endif
#if defined(RSK_TFT)
#undef RSK_TFT
#endif
#if defined(TFP410PAP)
#undef TFP410PAP
#endif
#if defined(TF043HV001A0)
#undef TF043HV001A0
#endif
#if defined(ATM0430D25)
#undef ATM0430D25
#endif
#if defined(FG040346DSSWBG03)
#undef FG040346DSSWBG03
#endif
#if defined(EP952)
#undef EP952
#endif
#if defined(LCD800x480)
#undef LCD800x480
#endif
#if defined(GR_PEACH_RSK_TFT)
#undef GR_PEACH_RSK_TFT
#endif
#if defined(LVDS_TO_HDMI)
#undef LVDS_TO_HDMI
#endif
#if defined(DVI_STICK)
#undef DVI_STICK
#endif
#if defined(RGB_TO_HDMI)
#undef RGB_TO_HDMI
#endif

#define LCD_LVDS                    0x8000

#define GR_PEACH_4_3INCH_SHIELD     (0x0000 | LCD_LVDS)
#define GR_PEACH_7_1INCH_SHIELD     (0x0001 | LCD_LVDS)
#define GR_PEACH_DISPLAY_SHIELD     (0x0002)
#define RSK_TFT                     (0x0003)
#define TFP410PAP                   (0x0004) /* HDMI and DVI */
#define TF043HV001A0                (0x0005)
#define ATM0430D25                  (0x0006)
#define FG040346DSSWBG03            (0x0007)
#define EP952                       (0x0008)
#define LCD_800x480                 (0x0009)

#define GR_PEACH_RSK_TFT            (RSK_TFT | LCD_LVDS)
#define LVDS_TO_HDMI                (TFP410PAP | LCD_LVDS)
#define DVI_STICK                   (TFP410PAP)

#if (1) /* GR-MANGO rev.B or later */
  #define RGB_TO_HDMI               (EP952)
#else /* GR-MANGO rev.A */
  #define RGB_TO_HDMI               (TFP410PAP)
#endif


#define NUM_OF_LAYER    3

typedef struct _lcd_t {
    uint16_t lcd_id;
    uint16_t lcd_size;  // SD_7INCH, SVGA, XGA, HD_720p
    uint16_t width;
    uint16_t height;
} lcd_t;

void mbed_lcd_start_display(display_t *dp);
bool mbed_lcd_graphic_init(lcd_t *lcd);
bool mbed_lcd_init(lcd_t *lcd);
void mbed_lcd_deinit(lcd_t *lcd);
bool mbed_lcd_is_initialzed(void);
// bool mbed_lcd_Graphics_Read_Setting(uint16_t layer, uint8_t *buf,
//     uint16_t stride, uint16_t format, uint16_t wr_rd, uint16_t hs, uint16_t vs, uint16_t hw, uint16_t vw);

#ifdef __cplusplus
};
#endif

#endif
