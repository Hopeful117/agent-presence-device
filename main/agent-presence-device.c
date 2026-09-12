/*
 * agent-presence-device.c - Application entry point
 *
 * Initializes the SH1106 OLED display and runs a "HELLO WORLD" demo.
 * All OLED driver details live in oled.c; font data lives in font.c.
 */

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"
#include "http_server.h"
#include "wifi_connection.h"
#include "button.h"
#include "presence.h"

static const char *TAG = "DEVLOG_HW";

void app_main(void)
{
   ESP_LOGI(TAG, "DevLog Hardware V0");

   ESP_ERROR_CHECK(oled_init());
   ESP_ERROR_CHECK(button_init());
   ESP_ERROR_CHECK(wifi_connect());
   ESP_ERROR_CHECK(http_server_start());

   /* Start with every pixel off. */
   oled_clear();
   ESP_ERROR_CHECK(oled_flush());

    bool was_pressed = false;

    while (true) {
        bool is_pressed = button_is_pressed();
        if (is_pressed && !was_pressed) {
            vTaskDelay(pdMS_TO_TICKS(30));

            if (button_is_pressed()) {
                presence_acknowledge();
            }
        }

        was_pressed = is_pressed;

        vTaskDelay(pdMS_TO_TICKS(20));
    }








}
