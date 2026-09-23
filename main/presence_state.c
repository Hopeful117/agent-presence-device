#include "presence_state.h"

void presence_state_init(PresenceStateModel *model)
{
    model->state = PRESENCE_STATE_IDLE;
    model->current_page = 0;
    model->page_count = 1;
}

void presence_state_message(PresenceStateModel *model, uint16_t page_count)
{
    model->state = PRESENCE_STATE_UNREAD;
    model->current_page = 0;
    model->page_count = page_count == 0 ? 1 : page_count;
}

void presence_state_short_press(PresenceStateModel *model)
{
    if (model->state == PRESENCE_STATE_IDLE) {
        return;
    }

    model->state = PRESENCE_STATE_READING;
    if (model->current_page + 1 < model->page_count) {
        model->current_page++;
    }
}

void presence_state_long_press(PresenceStateModel *model)
{
    model->state = PRESENCE_STATE_IDLE;
}

void presence_state_timeout(PresenceStateModel *model)
{
    if (model->state == PRESENCE_STATE_READING) {
        model->state = PRESENCE_STATE_IDLE;
    }
}

bool presence_state_is_reading(const PresenceStateModel *model)
{
    return model->state == PRESENCE_STATE_READING;
}
