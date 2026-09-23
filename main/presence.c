#include "presence.h"
#include "oled.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define READING_TIMEOUT_MS 10000
#define PRESENCE_MAX_PAGE_COUNT \
    ((PRESENCE_MAX_BODY_LINES + PRESENCE_BODY_LINES_PER_PAGE - 1) \
     / PRESENCE_BODY_LINES_PER_PAGE)

static SemaphoreHandle_t presence_mutex;
static PresenceStateModel presence_state;
static TickType_t presence_deadline;
static uint16_t presence_current_page;
static uint16_t presence_page_count;
static size_t presence_line_count;
static PresenceLine presence_lines[PRESENCE_MAX_BODY_LINES];
static char presence_source[PRESENCE_LINE_CHARS + 1];
static char presence_title[PRESENCE_LINE_CHARS + 1];
static char presence_body[PRESENCE_BODY_MAX_BYTES + 1];

static size_t utf8_safe_prefix(const char *text, size_t max_bytes)
{
    size_t length = strnlen(text, max_bytes);
    while (length > 0 && ((unsigned char) text[length - 1] & 0xC0U) == 0x80U) {
        length--;
    }
    return length;
}

static void copy_bounded(char *destination, size_t capacity, const char *source)
{
    if (source == NULL || capacity == 0) {
        if (capacity > 0) {
            destination[0] = '\0';
        }
        return;
    }

    size_t length = utf8_safe_prefix(source, capacity - 1);
    memcpy(destination, source, length);
    destination[length] = '\0';
}

static void presence_clear_locked(void)
{
    oled_clear();
    oled_flush();
}

static void presence_draw_locked(void)
{
    char line[PRESENCE_LINE_CHARS + 1];
    char page_indicator[12];
    size_t first_line = (size_t) presence_current_page * PRESENCE_BODY_LINES_PER_PAGE;

    oled_clear();
    oled_draw_text(0, 0, presence_source);
    oled_draw_text(0, 2, presence_title);
    for (size_t index = 0; index < PRESENCE_BODY_LINES_PER_PAGE; index++) {
        size_t line_index = first_line + index;
        line[0] = '\0';
        if (line_index < presence_line_count) {
            PresenceLine body_line = presence_lines[line_index];
            memcpy(line, &presence_body[body_line.start], body_line.length);
            line[body_line.length] = '\0';
        }
        oled_draw_text(0, (uint8_t) (4 + index), line);
    }

    if (presence_page_count > 1) {
        snprintf(page_indicator, sizeof(page_indicator), "%u/%u",
                 (unsigned) (presence_current_page + 1),
                 (unsigned) presence_page_count);
        oled_draw_text(0, 7, page_indicator);
    }
    oled_flush();
}

esp_err_t presence_init(void)
{
    presence_mutex = xSemaphoreCreateMutex();
    if (presence_mutex == NULL) {
        return ESP_ERR_NO_MEM;
    }
    presence_state_init(&presence_state);
    presence_deadline = 0;
    presence_current_page = 0;
    presence_page_count = 1;
    return ESP_OK;
}

void presence_show_message(const AgentMessage *message)
{
    if (message == NULL || presence_mutex == NULL) {
        return;
    }

    xSemaphoreTake(presence_mutex, portMAX_DELAY);
    copy_bounded(presence_source, sizeof(presence_source), message->source);
    copy_bounded(presence_title, sizeof(presence_title), message->title);
    copy_bounded(presence_body, sizeof(presence_body), message->body);
    presence_line_count = presence_layout_body(presence_body, presence_lines,
                                               PRESENCE_MAX_BODY_LINES);
    presence_page_count = (uint16_t) ((presence_line_count + PRESENCE_BODY_LINES_PER_PAGE - 1)
                                      / PRESENCE_BODY_LINES_PER_PAGE);
    if (presence_page_count == 0 || presence_page_count > PRESENCE_MAX_PAGE_COUNT) {
        presence_page_count = 1;
    }
    presence_state_message(&presence_state, presence_page_count);
    presence_current_page = presence_state.current_page;
    presence_page_count = presence_state.page_count;
    presence_deadline = 0;
    presence_draw_locked();
    xSemaphoreGive(presence_mutex);
}

void presence_next_page(void)
{
    if (presence_mutex == NULL) {
        return;
    }

    xSemaphoreTake(presence_mutex, portMAX_DELAY);
    if (presence_state.state != PRESENCE_STATE_IDLE) {
        presence_state_short_press(&presence_state);
        presence_current_page = presence_state.current_page;
        presence_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(READING_TIMEOUT_MS);
        presence_draw_locked();
    }
    xSemaphoreGive(presence_mutex);
}

void presence_acknowledge(void)
{
    if (presence_mutex == NULL) {
        return;
    }

    xSemaphoreTake(presence_mutex, portMAX_DELAY);
    presence_state_long_press(&presence_state);
    presence_deadline = 0;
    presence_clear_locked();
    xSemaphoreGive(presence_mutex);
}

void presence_update(void)
{
    if (presence_mutex == NULL) {
        return;
    }

    xSemaphoreTake(presence_mutex, portMAX_DELAY);
    if (presence_state_is_reading(&presence_state)
        && (int32_t) (xTaskGetTickCount() - presence_deadline) >= 0) {
        presence_state_timeout(&presence_state);
        presence_clear_locked();
    }
    xSemaphoreGive(presence_mutex);
}
