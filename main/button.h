//
// Created by ludo on 12/09/2026.
//

#ifndef AGENT_PRESENCE_DEVICE_BUTTON_H
#define AGENT_PRESENCE_DEVICE_BUTTON_H
#include "esp_err.h"
#include <stdbool.h>

esp_err_t button_init(void);
bool button_is_pressed(void);
#endif //AGENT_PRESENCE_DEVICE_BUTTON_H
