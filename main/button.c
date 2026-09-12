//
// Created by ludo on 12/09/2026.
//

#include "button.h"
#include "driver/gpio.h"
#include <stdbool.h>
#define BUTTON_GPIO GPIO_NUM_18




esp_err_t button_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    return gpio_config(&config);
}

bool button_is_pressed(void)
{
    return gpio_get_level(BUTTON_GPIO) == 0;
}