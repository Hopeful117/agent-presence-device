
#ifndef AGENT_PRESENCE_DEVICE_AGENT_MESSAGE_H
#define AGENT_PRESENCE_DEVICE_AGENT_MESSAGE_H

typedef enum {
    AGENT_INFO,
    AGENT_WARNING,
    AGENT_ATTENTION
} AgentMessageLevel;

typedef struct {
    const char *source;
    const char *title;
    const char *body;
    AgentMessageLevel level;
} AgentMessage;

#endif //AGENT_PRESENCE_DEVICE_AGENT_MESSAGE_H
