#include "notify/notify.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "notify";

static notify_config_t s_config;
static bool s_initialized;

static void url_encode(const char *src, char *dst, size_t dst_size)
{
    static const char *hex = "0123456789ABCDEF";
    size_t len = 0;
    while (*src && len + 3 < dst_size) {
        unsigned char c = (unsigned char)*src;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            dst[len++] = c;
        } else {
            dst[len++] = '%';
            dst[len++] = hex[c >> 4];
            dst[len++] = hex[c & 15];
        }
        src++;
    }
    dst[len] = '\0';
}

void notify_init(const notify_config_t *config)
{
    if (!config) return;
    memcpy(&s_config, config, sizeof(s_config));
    s_initialized = true;
    notify_mqtt_init(config);
}

const char *notify_get_bot_token(void)
{
    return s_initialized && s_config.telegram_bot_token ? s_config.telegram_bot_token : NULL;
}

void notify_send_alert(bool is_open)
{
    if (!s_initialized) return;

    notify_mqtt_publish(is_open);

    if (!s_config.telegram_bot_token || s_config.num_chats == 0) return;

    for (size_t i = 0; i < s_config.num_chats && i < 8; i++) {
        if (!s_config.telegram_chat_ids[i]) continue;

        char msg[256];
        const char *g = (i < 8 && s_config.greetings[i]) ? s_config.greetings[i] : "Hello";
        if (is_open) {
            snprintf(msg, sizeof(msg),
                     "%s, your door or window is OPEN! Please reply with 'away' or 'here'", g);
        } else {
            snprintf(msg, sizeof(msg), "%s, your door or window is CLOSED.", g);
        }

        char encoded[512];
        url_encode(msg, encoded, sizeof(encoded));

        char url[1024];
        snprintf(url, sizeof(url),
                 "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s",
                 s_config.telegram_bot_token, s_config.telegram_chat_ids[i], encoded);

        esp_http_client_config_t cfg = {
            .url = url,
            .method = HTTP_METHOD_GET,
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .crt_bundle_attach = esp_crt_bundle_attach,
        };
        esp_http_client_handle_t client = esp_http_client_init(&cfg);
        if (client) {
            esp_err_t err = esp_http_client_perform(client);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Alert sent to user %u", (unsigned)i + 1);
            } else {
                ESP_LOGE(TAG, "HTTP failed for user %u: %s", (unsigned)i + 1, esp_err_to_name(err));
            }
            esp_http_client_cleanup(client);
        }
    }
}

void notify_send_open(void)
{
    notify_send_alert(true);
}

void notify_send_closed(void)
{
    notify_send_alert(false);
}
