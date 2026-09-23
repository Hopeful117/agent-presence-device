#include "presence_state.h"

#include <assert.h>

int main(void)
{
    PresenceStateModel model;
    presence_state_init(&model);
    assert(model.state == PRESENCE_STATE_IDLE);

    presence_state_message(&model, 3);
    assert(model.state == PRESENCE_STATE_UNREAD);
    assert(model.current_page == 0);

    presence_state_short_press(&model);
    assert(model.state == PRESENCE_STATE_READING);
    assert(model.current_page == 1);

    presence_state_short_press(&model);
    assert(model.current_page == 2);
    presence_state_short_press(&model);
    assert(model.current_page == 2);

    presence_state_timeout(&model);
    assert(model.state == PRESENCE_STATE_IDLE);

    presence_state_message(&model, 1);
    presence_state_short_press(&model);
    assert(model.state == PRESENCE_STATE_READING);
    assert(model.current_page == 0);
    presence_state_long_press(&model);
    assert(model.state == PRESENCE_STATE_IDLE);

    presence_state_message(&model, 3);
    presence_state_long_press(&model);
    assert(model.state == PRESENCE_STATE_IDLE);

    presence_state_message(&model, 3);
    presence_state_short_press(&model);
    presence_state_message(&model, 1);
    assert(model.state == PRESENCE_STATE_UNREAD);
    assert(model.current_page == 0);
    assert(model.page_count == 1);
    presence_state_timeout(&model);
    assert(model.state == PRESENCE_STATE_UNREAD);

    return 0;
}
