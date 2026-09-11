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
#include "presence.h"
#include "agent_message.h"

static const char *TAG = "DEVLOG_HW";

void app_main(void)
{
   ESP_LOGI(TAG, "DevLog Hardware V0");

   ESP_ERROR_CHECK(oled_init());

   /* Start with every pixel off. */
   oled_clear();
   ESP_ERROR_CHECK(oled_flush());

    const AgentMessage message = {
    .source = "DEVLOG",
    .title = "BUILD OK",
    .body = "STORY 0119",
    .level = AGENT_INFO
};
    presence_show_message(&message);





   /* Keep the smoke test visible briefly, then power off the display. */
   sleep(30);
   oled_turn_off();
}
