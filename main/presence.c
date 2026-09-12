

#include "presence.h"
#include "oled.h"



void presence_show_message(const AgentMessage *message)
{

    if (message == NULL) {
        return;
    }
    oled_clear();
    oled_draw_text(0,0,message->source);
    oled_draw_text(0,2,message->title);
    oled_draw_text(0,4,message->body);


    oled_flush();
}
void presence_acknowledge(void)
{
    oled_clear();
    oled_flush();
}