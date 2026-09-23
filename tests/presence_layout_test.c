#include "presence_layout.h"

#include <assert.h>
#include <string.h>

static void assert_line(const char *body, size_t index, const char *expected)
{
    PresenceLine lines[32];
    size_t count = presence_layout_body(body, lines, 16);
    char actual[PRESENCE_LINE_CHARS + 1];

    assert(index < count);
    memcpy(actual, body + lines[index].start, lines[index].length);
    actual[lines[index].length] = '\0';
    assert(strcmp(actual, expected) == 0);
}

int main(void)
{
    PresenceLine lines[16];
    char maximum_body[PRESENCE_BODY_MAX_BYTES + 1];

    assert(presence_layout_body("", lines, 16) == 1);
    assert_line("short body", 0, "short body");
    assert_line("1234567890123456789012", 0, "123456789012345678901");
    assert_line("one two three four five six", 1, "five six");
    assert(presence_layout_body("one\ntwo", lines, 16) == 2);
    assert_line("one\ntwo", 1, "two");
    assert_line("multiple   spaces", 0, "multiple   spaces");
    assert_line("averylongwordthatexceedstheline", 0, "averylongwordthatexce");
    assert(presence_layout_body(
        "one two three four five six seven eight nine ten eleven twelve",
        lines, 16) >= 2);
    assert_line("prefix é suffix", 0, "prefix é suffix");

    memset(maximum_body, 'a', PRESENCE_BODY_MAX_BYTES);
    maximum_body[PRESENCE_BODY_MAX_BYTES] = '\0';
    assert(presence_layout_body(maximum_body, lines, 32) == 25);

    return 0;
}
