/*
 * agent-presence-device.c - Application entry point
 *
 * Initializes the SH1106 OLED display and runs a "HELLO WORLD" demo.
 * All OLED driver details live in oled.c; font data lives in font.c.
 */

#include <stdint.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_err.h"


#include "oled.h"
#include "agent_message.h"
#include "http_server.h"
#include "message_handler.h"
#include "wifi_connection.h"


static const char *TAG = "DEVLOG_HW";

void app_main(void)
{
   ESP_LOGI(TAG, "DevLog Hardware V0");

   ESP_ERROR_CHECK(oled_init());
   ESP_ERROR_CHECK(wifi_connect());
   ESP_ERROR_CHECK(http_server_start());

   /* Start with every pixel off. */
   oled_clear();
   ESP_ERROR_CHECK(oled_flush());








}
