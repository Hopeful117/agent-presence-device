//
// Created by ludo on 11/09/2026.
//

#ifndef AGENT_PRESENCE_DEVICE_PRESENCE_H
#define AGENT_PRESENCE_DEVICE_PRESENCE_H
#include "esp_err.h"
#include "agent_message.h"
#include "presence_layout.h"
#include "presence_state.h"

#define PRESENCE_LONG_PRESS_THRESHOLD_MS 1000

esp_err_t presence_init(void);
void presence_show_message(const AgentMessage *message);
void presence_acknowledge(void);
void presence_update(void);
void presence_next_page(void);

#endif //AGENT_PRESENCE_DEVICE_PRESENCE_H
