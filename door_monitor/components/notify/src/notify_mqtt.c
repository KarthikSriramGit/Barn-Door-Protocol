#ifdef CONFIG_DOOR_MONITOR_MQTT

#include "notify/notify.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "notify_mqtt";

static esp_mqtt_client_handle_t s_client;
static char s_topic[128];
static bool s_mqtt_ready;

void notify_mqtt_init(const notify_config_t *config)
{
    if (!config || !config->mqtt_uri || !config->mqtt_topic) return;
    s_mqtt_ready = false;
    strncpy(s_topic, config->mqtt_topic, sizeof(s_topic) - 1);
    s_topic[sizeof(s_topic) - 1] = '\0';

    esp_mqtt_client_config_t mqtt_cfg = { 0 };
    mqtt_cfg.broker.address.uri = config->mqtt_uri;

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_client) return;
    esp_err_t err = esp_mqtt_client_start(s_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MQTT start failed: %s", esp_err_to_name(err));
        return;
    }
    s_mqtt_ready = true;
    ESP_LOGI(TAG, "MQTT started");
}

void notify_mqtt_publish(bool is_open)
{
    if (!s_mqtt_ready || !s_client) return;
    const char *payload = is_open ? "open" : "closed";
    int id = esp_mqtt_client_publish(s_client, s_topic, payload, 0, 0, 0);
    if (id < 0) {
        ESP_LOGW(TAG, "MQTT publish failed");
    }
}

#else

#include "notify/notify.h"

void notify_mqtt_init(const notify_config_t *config)
{
    (void)config;
}

void notify_mqtt_publish(bool is_open)
{
    (void)is_open;
}

#endif
