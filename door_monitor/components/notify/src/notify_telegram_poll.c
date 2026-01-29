#ifdef CONFIG_DOOR_MONITOR_TELEGRAM_POLL

#include "notify/notify.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int eq_ignore(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}

static const char *TAG = "notify_tg_poll";

static void (*s_away_cb)(bool away) = NULL;
static int64_t s_offset = 0;

static void parse_updates(const char *json)
{
    cJSON *root = cJSON_Parse(json);
    if (!root) return;
    cJSON *res = cJSON_GetObjectItem(root, "result");
    if (!res || !cJSON_IsArray(res)) {
        cJSON_Delete(root);
        return;
    }
    int n = cJSON_GetArraySize(res);
    for (int i = 0; i < n; i++) {
        cJSON *u = cJSON_GetArrayItem(res, i);
        cJSON *id = cJSON_GetObjectItem(u, "update_id");
        if (id && cJSON_IsNumber(id)) {
            int64_t uid = (int64_t)id->valuedouble;
            if (uid > s_offset) s_offset = uid;
        }
        cJSON *msg = cJSON_GetObjectItem(u, "message");
        if (!msg) continue;
        cJSON *text = cJSON_GetObjectItem(msg, "text");
        if (!text || !cJSON_IsString(text)) continue;
        const char *t = text->valuestring;
        if (eq_ignore(t, "away") && s_away_cb) s_away_cb(true);
        else if (eq_ignore(t, "here") && s_away_cb) s_away_cb(false);
    }
    cJSON_Delete(root);
}

static int http_get_body(const char *url, char *buf, size_t buf_size)
{
    esp_http_client_config_t cfg = {
        .url = url,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) return -1;
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    if (esp_http_client_open(client, 0) != ESP_OK) {
        esp_http_client_cleanup(client);
        return -1;
    }
    esp_http_client_fetch_headers(client);
    int total = 0;
    int r;
    while ((r = esp_http_client_read(client, buf + total, (int)(buf_size - total - 1))) > 0) {
        total += r;
        if (total >= (int)buf_size - 1) break;
    }
    buf[total] = '\0';
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return total;
}

static void notify_poll_task(void *arg)
{
    (void)arg;
    char url[280];
    char buf[2048];
    const char *tok;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(CONFIG_DOOR_MONITOR_TELEGRAM_POLL_MS));
        tok = notify_get_bot_token();
        if (!tok || !s_away_cb) continue;
        snprintf(url, sizeof(url),
                 "https://api.telegram.org/bot%s/getUpdates?offset=%lld&timeout=0",
                 tok, (long long)(s_offset + 1));
        if (http_get_body(url, buf, sizeof(buf)) > 0)
            parse_updates(buf);
    }
}

void notify_set_away_callback(void (*cb)(bool away))
{
    s_away_cb = cb;
}

void notify_start_poll_task(void)
{
    if (!notify_get_bot_token()) return;
    xTaskCreate(notify_poll_task, "tg_poll", 4096, NULL, 3, NULL);
    ESP_LOGI(TAG, "Telegram poll task started");
}

#else

#include "notify/notify.h"

void notify_set_away_callback(void (*cb)(bool away))
{
    (void)cb;
}

void notify_start_poll_task(void)
{
}

#endif
