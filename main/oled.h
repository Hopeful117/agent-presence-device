/*
 * oled.h - SH1106 OLED driver (SPI, 128x64)
 *
 * Public API for display initialization, framebuffer management, and rendering.
 * The framebuffer is owned by this module and accessed through the functions below.
 */

#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#include "esp_err.h"

#include "font.h"

/* ---- Display geometry ---- */
#define OLED_WIDTH   128
#define OLED_HEIGHT   64
#define OLED_PAGES   (OLED_HEIGHT / 8)   /* 8 pages, each 8 pixels tall */

/* ---- Lifecycle ---- */
esp_err_t oled_init(void);
void      oled_turn_off(void);

/* ---- Framebuffer ---- */
void      oled_clear(void);
esp_err_t oled_flush(void);

/* ---- Rendering ---- */
void oled_draw_char(uint8_t x, uint8_t page, const uint8_t glyph[GLYPH_WIDTH]);
void oled_set_pixel(uint8_t x, uint8_t y);
void oled_draw_text(uint8_t x, uint8_t page, const char *text);

#endif /* OLED_H */
