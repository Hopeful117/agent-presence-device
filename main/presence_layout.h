#ifndef AGENT_PRESENCE_DEVICE_PRESENCE_LAYOUT_H
#define AGENT_PRESENCE_DEVICE_PRESENCE_LAYOUT_H

#include <stddef.h>
#include <stdint.h>

#define PRESENCE_LINE_CHARS 21
#define PRESENCE_BODY_LINES_PER_PAGE 3
#define PRESENCE_BODY_MAX_BYTES 511
#define PRESENCE_MAX_BODY_LINES (PRESENCE_BODY_MAX_BYTES + 1)

typedef struct {
    uint16_t start;
    uint8_t length;
} PresenceLine;

size_t presence_layout_body(const char *body, PresenceLine *lines, size_t max_lines);

#endif
