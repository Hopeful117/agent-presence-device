#include "presence_layout.h"

#include <stddef.h>

static size_t utf8_sequence_length(const unsigned char *text)
{
    if ((text[0] & 0x80U) == 0) {
        return 1;
    }
    if ((text[0] & 0xE0U) == 0xC0U && (text[1] & 0xC0U) == 0x80U) {
        return 2;
    }
    if ((text[0] & 0xF0U) == 0xE0U && (text[1] & 0xC0U) == 0x80U
        && (text[2] & 0xC0U) == 0x80U) {
        return 3;
    }
    if ((text[0] & 0xF8U) == 0xF0U && (text[1] & 0xC0U) == 0x80U
        && (text[2] & 0xC0U) == 0x80U && (text[3] & 0xC0U) == 0x80U) {
        return 4;
    }
    return 1;
}

size_t presence_layout_body(const char *body, PresenceLine *lines, size_t max_lines)
{
    if (lines == NULL || max_lines == 0) {
        return 0;
    }
    if (body == NULL || body[0] == '\0') {
        lines[0] = (PresenceLine) { .start = 0, .length = 0 };
        return 1;
    }

    size_t line_count = 0;
    size_t line_start = 0;
    size_t line_length = 0;
    size_t position = 0;
    size_t last_space = SIZE_MAX;

    while (body[position] != '\0' && line_count < max_lines) {
        unsigned char character = (unsigned char) body[position];
        if (character == '\n') {
            lines[line_count++] = (PresenceLine) {
                .start = (uint16_t) line_start,
                .length = (uint8_t) line_length
            };
            position++;
            line_start = position;
            line_length = 0;
            last_space = SIZE_MAX;
            continue;
        }

        size_t sequence_length = utf8_sequence_length((const unsigned char *) &body[position]);
        if (line_length > 0 && line_length + sequence_length > PRESENCE_LINE_CHARS) {
            if (last_space != SIZE_MAX) {
                lines[line_count++] = (PresenceLine) {
                    .start = (uint16_t) line_start,
                    .length = (uint8_t) (last_space - line_start)
                };
                position = last_space + 1;
            } else {
                lines[line_count++] = (PresenceLine) {
                    .start = (uint16_t) line_start,
                    .length = (uint8_t) line_length
                };
            }
            line_start = position;
            line_length = 0;
            last_space = SIZE_MAX;
            continue;
        }

        if (character == ' ' || character == '\t') {
            if (line_length == 0) {
                position++;
                line_start = position;
                continue;
            }
            last_space = position;
        }
        line_length += sequence_length;
        position += sequence_length;
    }

    if (line_count < max_lines && (line_length > 0 || line_count == 0
                                   || body[position - 1] == '\n')) {
        lines[line_count++] = (PresenceLine) {
            .start = (uint16_t) line_start,
            .length = (uint8_t) line_length
        };
    }
    return line_count;
}
