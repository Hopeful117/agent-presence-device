//
// Created by ludo on 12/09/2026.
//

#include "http_server.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include <stdbool.h>
#include <string.h>
#include "agent_message.h"
#include "message_handler.h"

static esp_err_t message_post_handler(httpd_req_t *req);
static bool parse_level(const char *value,AgentMessageLevel *level);

esp_err_t http_server_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    esp_err_t err= httpd_start(&server, &config);

    if (err != ESP_OK) {
        return err;
    }
    httpd_uri_t message_uri = {
        .uri = "/message",
        .method = HTTP_POST,
        .handler = message_post_handler,
        .user_ctx = NULL
    };
    err = httpd_register_uri_handler(server, &message_uri);

    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}
static esp_err_t message_post_handler(httpd_req_t *req)
{
    char buffer[512];

    if (req->content_len >= sizeof(buffer)) {
        return httpd_resp_send_err(
            req,
            HTTPD_413_CONTENT_TOO_LARGE,
            "Request body too large"
        );
    }

    int total_received = 0;

    while (total_received < req->content_len) {

        int received = httpd_req_recv(
            req,
            buffer + total_received,
            req->content_len - total_received
        );

        if (received <= 0) {
            return ESP_FAIL;
        }

        total_received += received;
    }

    buffer[total_received] = '\0';
    cJSON *json = cJSON_Parse(buffer);

    if (json == NULL) {
        return httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid JSON"
        );
    }
    cJSON *source = cJSON_GetObjectItem(json, "source");
    cJSON *title = cJSON_GetObjectItem(json, "title");
    cJSON *body = cJSON_GetObjectItem(json, "body");
    cJSON *level = cJSON_GetObjectItem(json, "level");

    if (!cJSON_IsString(source)
    || !cJSON_IsString(title)
    || !cJSON_IsString(body)
    || !cJSON_IsString(level)) {

        cJSON_Delete(json);

        return httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Missing or invalid fields"
        );
    }
    AgentMessageLevel message_level;

    if (!parse_level(level->valuestring, &message_level)) {
        cJSON_Delete(json);

        return httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid level"
        );
    }
    AgentMessage message = {
        .source = source->valuestring,
        .title = title->valuestring,
        .body = body->valuestring,
        .level = message_level
    };

    message_handler_handle(&message);

    cJSON_Delete(json);



    return httpd_resp_send(
        req,
        "MESSAGE RECEIVED",
        HTTPD_RESP_USE_STRLEN
    );
}
static bool parse_level(
    const char *value,
    AgentMessageLevel *level
)
{
    if (strcmp(value, "INFO") == 0) {
        *level = AGENT_INFO;
        return true;
    }

    if (strcmp(value, "WARNING") == 0) {
        *level = AGENT_WARNING;
        return true;
    }

    if (strcmp(value, "ATTENTION") == 0) {
        *level = AGENT_ATTENTION;
        return true;
    }

    return false;
}