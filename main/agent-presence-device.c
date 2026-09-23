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
   ESP_ERROR_CHECK(presence_init());
   ESP_ERROR_CHECK(button_init());
   ESP_ERROR_CHECK(wifi_connect());
   ESP_ERROR_CHECK(http_server_start());

   /* Start with every pixel off. */
   presence_acknowledge();

    bool was_pressed = false;
    TickType_t press_started = 0;

    while (true) {
        presence_update();
        bool is_pressed = button_is_pressed();
        if (is_pressed != was_pressed) {
            vTaskDelay(pdMS_TO_TICKS(30));

            bool stable_pressed = button_is_pressed();
            if (stable_pressed != was_pressed) {
                if (stable_pressed) {
                    press_started = xTaskGetTickCount();
                } else {
                    TickType_t duration = xTaskGetTickCount() - press_started;
                    if (duration >= pdMS_TO_TICKS(PRESENCE_LONG_PRESS_THRESHOLD_MS)) {
                        presence_acknowledge();
                    } else {
                        presence_next_page();
                    }
                }
                was_pressed = stable_pressed;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }








}
