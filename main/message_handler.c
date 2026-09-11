//
// Created by ludo on 11/09/2026.
//

#include "message_handler.h"

#include "presence.h"

void message_handler_handle(const AgentMessage *message) {
    presence_show_message(message);
}
