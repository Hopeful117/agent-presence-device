/*
 * DevLog Hardware V0 — SH1106 OLED via SPI
 *
 * This file drives a 128x64 SH1106 OLED over SPI on ESP32-S3-DEVKIT-LIPO.
 *
 * Architecture overview (traceable path):
 *   app_main
 *     -> oled_draw_char   (writes pixels into oled_buffer)
 *     -> oled_flush       (iterates pages, calls oled_write_page)
 *     -> oled_write_page  (sets page address + column offset, sends data)
 *     -> oled_send_data   (sets DC=1, SPI transmit)
 *
 * Framebuffer layout:
 *   oled_buffer[128 * 8] = 1024 bytes
 *   Organized as 8 pages (page 0..7), each page holding 128 columns.
 *   Byte index = page * 128 + column.
 *   Within one byte, bit 0 = top pixel, bit 7 = bottom pixel of that 8px group.
 *
 * SH1106 pages:
 *   The SH1106 maps memory in 8-pixel-tall horizontal bands called "pages".
 *   Page 0 covers pixels y=0..7, page 1 covers y=8..15, etc.
 *   Each byte written to a page sets a vertical column of 8 pixels at once.
 *   This is why OLED_WIDTH * (OLED_HEIGHT/8) bytes = 1024 bytes total.
 *
 * SPI command vs data (DC pin):
 *   DC (Data/Command) pin selects what the SH1106 interprets:
 *     DC=0 -> next byte is a command (set page, set column, display on/off, etc.)
 *     DC=1 -> next byte(s) are pixel data (written into the current page/column)
 *   RES (Reset) pin:
 *     Active-low hardware reset. Pulses low for 10ms to initialize the SH1106.
 *   CS (Chip Select) pin:
 *     Active-low select. The SPI driver manages this automatically via spics_io_num.
 *
 * Why MISO is unused:
 *   The SH1106 is write-only from the MCU's perspective. It never sends data back.
 *   Setting miso_io_num = -1 avoids allocating a receive DMA channel.
 *
 * Why GPIO19/GPIO20 are avoided:
 *   On ESP32-S3-DevKit-1, GPIO19 and GPIO20 are used for USB-Serial-JTAG.
 *   Using them for general GPIO would break the serial console / flashing.
 *
 * SH1106 +2 column offset:
 *   The SH1106 has an internal RAM of 132 columns. The visible 128-column panel
 *   is physically centered, leaving 2 unused columns on each side. The visible
 *   area starts at internal column 2, so every page write must begin at column 2.
 *   This is why oled_write_page sends 0x02 (low nibble) and 0x10 (high nibble).
 *
 * Current 5x7 glyph limitations:
 *   - Glyphs are defined as 5 bytes (5 columns), each byte = 8 vertical bits.
 *   - Only the bottom 7 bits are used for the visible glyph; the 8th row is padding.
 *   - No baseline alignment, no kerning, no proportional spacing.
 *   - Fixed 1-pixel spacing between characters (x += 6 per glyph).
 *   - Only H, E, L, O, W, R, D are defined — not a general-purpose font.
 *   - Character placement writes directly to the buffer; no abstraction layer.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <unistd.h>

/* ---- GPIO assignments (SPI3 / SPI2_HOST) ---- */
#define OLED_PIN_MOSI  11   /* SPI data out to SH1106 SDA */
#define OLED_PIN_CLK   12   /* SPI clock to SH1106 SCK  */
#define OLED_PIN_CS     8   /* Chip select (active low, managed by SPI driver) */
#define OLED_PIN_DC     9   /* Data/Command select (0=cmd, 1=data) */
#define OLED_PIN_RES   10   /* Hardware reset (active low pulse) */

/* ---- Display geometry ---- */
#define OLED_WIDTH   128
#define OLED_HEIGHT   64
#define OLED_PAGES   (OLED_HEIGHT / 8)   /* 8 pages, each 8 pixels tall */
#define GLYPH_WIDTH    5                  /* columns per character glyph */
#define GLYPH_SPACING  6                  /* horizontal advance per glyph (5 px + 1 gap) */

/* ---- Framebuffer ----
 * Layout: 8 pages, each page = 128 bytes (one byte per column).
 * Byte at index [page * OLED_WIDTH + column] controls 8 vertical pixels.
 * Bit 0 = topmost pixel in the group, bit 7 = bottommost.
 */
static uint8_t oled_buffer[OLED_WIDTH * OLED_PAGES];

static const char *TAG = "DEVLOG_HW";
static spi_device_handle_t oled_handle;

/* Forward declarations */
static esp_err_t oled_gpio_init(void);
static esp_err_t oled_spi_init(void);
static esp_err_t oled_device_init(void);
static void oled_hardware_reset(void);
static esp_err_t oled_send_command(uint8_t command);
static esp_err_t oled_init_sh1106(void);
static esp_err_t oled_send_data(const uint8_t *data, size_t length);
static esp_err_t oled_test_pattern(void);
static esp_err_t oled_flush(void);
static void oled_turn_off(void);
static void oled_draw_char(uint8_t x, uint8_t page, const uint8_t glyph[GLYPH_WIDTH]);

/* ---- Minimal 5x7 font (column-major, 5 bytes per glyph) ----
 * Each glyph is 5 columns wide. Within each column byte, bit 0 = top pixel.
 * Only the characters needed for "HELLO WORLD" are defined.
 */
static const uint8_t FONT_H[GLYPH_WIDTH] = { 0x7F, 0x08, 0x08, 0x08, 0x7F };
static const uint8_t FONT_E[GLYPH_WIDTH] = { 0x7F, 0x49, 0x49, 0x49, 0x41 };
static const uint8_t FONT_L[GLYPH_WIDTH] = { 0x7F, 0x40, 0x40, 0x40, 0x40 };
static const uint8_t FONT_O[GLYPH_WIDTH] = { 0x3E, 0x41, 0x41, 0x41, 0x3E };
static const uint8_t FONT_W[GLYPH_WIDTH] = { 0x7F, 0x20, 0x18, 0x20, 0x7F };
static const uint8_t FONT_R[GLYPH_WIDTH] = { 0x7F, 0x09, 0x19, 0x29, 0x46 };
static const uint8_t FONT_D[GLYPH_WIDTH] = { 0x7F, 0x41, 0x41, 0x22, 0x1C };

/* ====================================================================
 * Application entry point
 * ==================================================================== */
void app_main(void)
{
   ESP_LOGI(TAG, "DevLog Hardware V0");

   ESP_ERROR_CHECK(oled_gpio_init());
   ESP_LOGI(TAG, "OLED GPIO initialized");

   ESP_ERROR_CHECK(oled_spi_init());
   ESP_LOGI(TAG, "SPI bus initialized");

   ESP_ERROR_CHECK(oled_device_init());
   ESP_LOGI(TAG, "OLED SPI device registered");

   oled_hardware_reset();
   ESP_LOGI(TAG, "OLED hardware reset complete");

   ESP_ERROR_CHECK(oled_init_sh1106());
   ESP_LOGI(TAG, "SH1106 initialized");

   /* Fill framebuffer with all-white (all bits set = all pixels on) */
   memset(oled_buffer, 0xFF, sizeof(oled_buffer));
   ESP_ERROR_CHECK(oled_flush());

   /* Draw "HELLO WORLD" at page 3 (vertical center of 64px display) */
   uint8_t x = 10;
   uint8_t page = 3;

   oled_draw_char(x, page, FONT_H);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_E);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_L);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_L);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_O);  x += 10;  /* extra space between words */

   oled_draw_char(x, page, FONT_W);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_O);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_R);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_L);  x += GLYPH_SPACING;
   oled_draw_char(x, page, FONT_D);

   ESP_ERROR_CHECK(oled_flush());

   /* Display stays on for 20 seconds, then powers off */
   sleep(20);
   oled_turn_off();
}

/* ====================================================================
 * Low-level hardware init
 * ==================================================================== */

/* Configure DC and RES as output GPIOs (CS is managed by SPI driver) */
static esp_err_t oled_gpio_init(void)
{
   gpio_config_t config = {
      .pin_bit_mask = (1ULL << OLED_PIN_DC) | (1ULL << OLED_PIN_RES),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE
   };

   return gpio_config(&config);
}

/* Initialize SPI2_HOST bus in half-duplex (TX only, MISO not connected) */
static esp_err_t oled_spi_init(void)
{
   spi_bus_config_t bus_config = {
      .mosi_io_num = OLED_PIN_MOSI,
      .miso_io_num = -1,            /* SH1106 is write-only; no MISO needed */
      .sclk_io_num = OLED_PIN_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
   };

   return spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
}

/* Register SH1106 as an SPI device on the bus */
static esp_err_t oled_device_init(void)
{
   spi_device_interface_config_t device_config = {
      .clock_speed_hz = 1 * 1000 * 1000,  /* 1 MHz SPI clock */
      .mode = 0,                           /* SPI mode 0: CPOL=0, CPHA=0 */
      .spics_io_num = OLED_PIN_CS,         /* CS driven by SPI driver */
      .queue_size = 1,
   };

   return spi_bus_add_device(SPI2_HOST, &device_config, &oled_handle);
}

/* Pulse RES low for 10ms to reset the SH1106 */
static void oled_hardware_reset(void)
{
   gpio_set_level(OLED_PIN_RES, 0);
   vTaskDelay(pdMS_TO_TICKS(10));
   gpio_set_level(OLED_PIN_RES, 1);
   vTaskDelay(pdMS_TO_TICKS(10));
}

/* ====================================================================
 * SPI data transfer
 * ==================================================================== */

/* Send one command byte: DC=0 tells SH1106 this is a command */
static esp_err_t oled_send_command(uint8_t command)
{
   gpio_set_level(OLED_PIN_DC, 0);

   spi_transaction_t transaction = {
      .length = 8,
      .tx_buffer = &command,
   };

   return spi_device_transmit(oled_handle, &transaction);
}

/* Send pixel data bytes: DC=1 tells SH1106 this is display data */
static esp_err_t oled_send_data(const uint8_t *data, size_t length)
{
   gpio_set_level(OLED_PIN_DC, 1);

   spi_transaction_t transaction = {
      .length = length * 8,
      .tx_buffer = data,
   };

   return spi_device_transmit(oled_handle, &transaction);
}

/* ====================================================================
 * SH1106 initialization sequence
 * ==================================================================== */

static esp_err_t oled_init_sh1106(void)
{
   static const uint8_t init_sequence[] = {
      0xAE,       /* Display OFF (prevents garbage during init) */
      0xD5, 0x80, /* Set clock divide ratio / oscillator frequency */
      0xA8, 0x3F, /* Multiplex ratio = 64 (64 rows) */
      0xD3, 0x00, /* Display offset = 0 (no vertical shift) */
      0x40,       /* Start line = 0 */
      0xAD, 0x8B, /* DC/DC charge pump (0x8B = enabled) */
      0xA1,       /* Segment remap: column 0 = left */
      0xC8,       /* COM scan direction: remapped (bottom-to-top) */
      0xDA, 0x12, /* COM pins hardware config (sequential, no remap) */
      0x81, 0xFF, /* Set contrast to maximum */
      0xD9, 0x1F, /* Pre-charge period */
      0xDB, 0x40, /* VCOMH deselect level */
      0x33,       /* VPP (charge pump voltage) */
      0xA6,       /* Normal display (not inverted) */
      0xA4        /* Display from RAM (not entire display on) */
   };

   for (size_t i = 0; i < sizeof(init_sequence); i++) {
      esp_err_t err = oled_send_command(init_sequence[i]);
      if (err != ESP_OK) {
         return err;
      }
   }

   vTaskDelay(pdMS_TO_TICKS(100));

   /* Display ON */
   return oled_send_command(0xAF);
}

/* ====================================================================
 * Framebuffer operations
 * ==================================================================== */

/* Write one page (8 pixel rows) of 128 bytes to the SH1106.
 * SH1106 internal RAM is 132 columns wide; the visible panel starts at
 * column 2, so we send 0x02 (low nibble) + 0x10 (high nibble) to set
 * the starting column address. */
static esp_err_t oled_write_page(uint8_t page, const uint8_t *data)
{
   ESP_ERROR_CHECK(oled_send_command(0xB0 | page));  /* Set page address */

   /* SH1106 column offset: visible area starts at internal column 2 */
   ESP_ERROR_CHECK(oled_send_command(0x02));          /* Column address low nibble */
   ESP_ERROR_CHECK(oled_send_command(0x10));          /* Column address high nibble */

   return oled_send_data(data, OLED_WIDTH);
}

/* Flush entire framebuffer to display: iterate all 8 pages */
static esp_err_t oled_flush(void)
{
   for (uint8_t page = 0; page < OLED_PAGES; page++) {
      ESP_ERROR_CHECK(
         oled_write_page(page, &oled_buffer[page * OLED_WIDTH])
      );
   }

   return ESP_OK;
}

/* Turn display off by sending the Display OFF command (0xAE) */
static void oled_turn_off(void)
{
   ESP_ERROR_CHECK(oled_send_command(0xAE));
}

/* Draw a single 5-column glyph into the framebuffer.
 * Bounds check: skip if page is out of range or glyph would overflow width. */
static void oled_draw_char(uint8_t x, uint8_t page, const uint8_t glyph[GLYPH_WIDTH])
{
   if (page >= OLED_PAGES || x + GLYPH_WIDTH >= OLED_WIDTH) {
      return;
   }

   for (uint8_t column = 0; column < GLYPH_WIDTH; column++) {
      oled_buffer[page * OLED_WIDTH + x + column] = glyph[column];
   }
}

/* ====================================================================
 * Test / debug helper
 * ==================================================================== */

/* Fill page 0 with an alternating-bit pattern (0xAA) for visual debug */
static esp_err_t oled_test_pattern(void)
{
   uint8_t pattern[OLED_WIDTH];

   for (size_t i = 0; i < sizeof(pattern); i++) {
      pattern[i] = 0xAA;
   }

   ESP_ERROR_CHECK(oled_send_command(0xB0));  /* Page 0 */
   ESP_ERROR_CHECK(oled_send_command(0x02));  /* Column low nibble (SH1106 offset) */
   ESP_ERROR_CHECK(oled_send_command(0x10));  /* Column high nibble */

   return oled_send_data(pattern, sizeof(pattern));
}
