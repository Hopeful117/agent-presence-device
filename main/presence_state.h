#ifndef AGENT_PRESENCE_DEVICE_PRESENCE_STATE_H
#define AGENT_PRESENCE_DEVICE_PRESENCE_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PRESENCE_STATE_IDLE,
    PRESENCE_STATE_UNREAD,
    PRESENCE_STATE_READING
} PresenceState;

typedef struct {
    PresenceState state;
    uint16_t current_page;
    uint16_t page_count;
} PresenceStateModel;

void presence_state_init(PresenceStateModel *model);
void presence_state_message(PresenceStateModel *model, uint16_t page_count);
void presence_state_short_press(PresenceStateModel *model);
void presence_state_long_press(PresenceStateModel *model);
void presence_state_timeout(PresenceStateModel *model);
bool presence_state_is_reading(const PresenceStateModel *model);

#endif
