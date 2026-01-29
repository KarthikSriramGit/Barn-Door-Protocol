#include "app_config.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#define NVS_NAMESPACE "door_monitor"
#define NVS_KEY_SSID "wifi_ssid"
#define NVS_KEY_PASS "wifi_pass"
#define NVS_KEY_TG_TOKEN "tg_token"
#define NVS_KEY_TG_NUM "tg_num_chats"
#define NVS_KEY_TG_CHAT "tg_chat_%d"
#define NVS_KEY_TG_GREET "tg_greet_%d"
#define NVS_KEY_ZONE_COUNT "zone_count"
#define NVS_KEY_ZONE_GPIO "zone_%d_gpio"
#define NVS_KEY_ZONE_LABEL "zone_%d_label"
#define NVS_KEY_DEBOUNCE_MS "debounce_ms"
#define NVS_KEY_ALERT_MS "alert_interval_ms"
#define NVS_KEY_LED_GPIO "led_gpio"
#define NVS_KEY_AWAY "away_mode"
#define NVS_KEY_QUIET_EN "quiet_en"
#define NVS_KEY_QUIET_START "quiet_start"
#define NVS_KEY_QUIET_END "quiet_end"
#define NVS_KEY_MQTT_URI "mqtt_uri"
#define NVS_KEY_MQTT_TOPIC "mqtt_topic"

static const char *TAG = "app_config";

#ifndef APP_CONFIG_DEFAULT_SSID
#define APP_CONFIG_DEFAULT_SSID "Ace & King of Hearts"
#endif
#ifndef APP_CONFIG_DEFAULT_PASS
#define APP_CONFIG_DEFAULT_PASS "PASSWORD"
#endif
#ifndef APP_CONFIG_DEFAULT_TG_TOKEN
#define APP_CONFIG_DEFAULT_TG_TOKEN "TELEGRAM_TOKEN"
#endif
#ifndef APP_CONFIG_DEFAULT_CHAT_1
#define APP_CONFIG_DEFAULT_CHAT_1 "9999999999"
#endif
#ifndef APP_CONFIG_DEFAULT_CHAT_2
#define APP_CONFIG_DEFAULT_CHAT_2 "9999999999"
#endif
#ifndef APP_CONFIG_DEFAULT_GREET_1
#define APP_CONFIG_DEFAULT_GREET_1 "Hello Mr Stark"
#endif
#ifndef APP_CONFIG_DEFAULT_GREET_2
#define APP_CONFIG_DEFAULT_GREET_2 "Hello Mrs Suseendran"
#endif

static void strncpy_safe(char *dst, const char *src, size_t n)
{
    if (n == 0) return;
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
}

void app_config_set_defaults(app_config_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    strncpy_safe(out->wifi.ssid, APP_CONFIG_DEFAULT_SSID, sizeof(out->wifi.ssid));
    strncpy_safe(out->wifi.password, APP_CONFIG_DEFAULT_PASS, sizeof(out->wifi.password));
    strncpy_safe(out->notify.token, APP_CONFIG_DEFAULT_TG_TOKEN, sizeof(out->notify.token));
    out->notify.num_chats = 2;
    strncpy_safe(out->notify.chat_ids[0], APP_CONFIG_DEFAULT_CHAT_1, sizeof(out->notify.chat_ids[0]));
    strncpy_safe(out->notify.chat_ids[1], APP_CONFIG_DEFAULT_CHAT_2, sizeof(out->notify.chat_ids[1]));
    strncpy_safe(out->notify.greetings[0], APP_CONFIG_DEFAULT_GREET_1, sizeof(out->notify.greetings[0]));
    strncpy_safe(out->notify.greetings[1], APP_CONFIG_DEFAULT_GREET_2, sizeof(out->notify.greetings[1]));

    out->sensor.zone_count = 1;
    out->sensor.debounce_ms = 50;
    out->sensor.alert_interval_ms = 60000;
    out->sensor.led_gpio = GPIO_NUM_2;
    out->sensor.zones[0].gpio = GPIO_NUM_4;
    strncpy_safe(out->sensor.zones[0].label, "door", sizeof(out->sensor.zones[0].label));

    out->state.away_mode = true;
    out->state.quiet_enabled = false;
    out->state.quiet_start_h = 22;
    out->state.quiet_end_h = 7;
}

static bool load_str(nvs_handle_t h, const char *key, char *buf, size_t size)
{
    size_t len = size;
    return nvs_get_str(h, key, buf, &len) == ESP_OK;
}

static bool load_u8(nvs_handle_t h, const char *key, uint8_t *v)
{
    return nvs_get_u8(h, key, v) == ESP_OK;
}

static bool load_u32(nvs_handle_t h, const char *key, uint32_t *v)
{
    return nvs_get_u32(h, key, v) == ESP_OK;
}

void app_config_load(app_config_t *out)
{
    if (!out) return;
    app_config_set_defaults(out);

    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK) return;

    char k[32];
    load_str(h, NVS_KEY_SSID, out->wifi.ssid, sizeof(out->wifi.ssid));
    load_str(h, NVS_KEY_PASS, out->wifi.password, sizeof(out->wifi.password));
    load_str(h, NVS_KEY_TG_TOKEN, out->notify.token, sizeof(out->notify.token));

    uint8_t n;
    if (load_u8(h, NVS_KEY_ZONE_COUNT, &n) && n > 0 && n <= APP_CONFIG_MAX_ZONES)
        out->sensor.zone_count = n;
    for (int i = 0; i < out->sensor.zone_count && i < APP_CONFIG_MAX_ZONES; i++) {
        snprintf(k, sizeof(k), NVS_KEY_ZONE_GPIO, i);
        uint32_t gpio;
        if (load_u32(h, k, &gpio)) out->sensor.zones[i].gpio = (gpio_num_t)gpio;
        snprintf(k, sizeof(k), NVS_KEY_ZONE_LABEL, i);
        load_str(h, k, out->sensor.zones[i].label, sizeof(out->sensor.zones[i].label));
    }

    if (load_u8(h, NVS_KEY_TG_NUM, &n) && n > 0) {
        size_t nc = (size_t)n;
        if (nc > APP_CONFIG_MAX_CHATS) nc = APP_CONFIG_MAX_CHATS;
        out->notify.num_chats = nc;
    }
    for (size_t i = 0; i < out->notify.num_chats && i < APP_CONFIG_MAX_CHATS; i++) {
        snprintf(k, sizeof(k), NVS_KEY_TG_CHAT, (int)i);
        load_str(h, k, out->notify.chat_ids[i], sizeof(out->notify.chat_ids[i]));
        snprintf(k, sizeof(k), NVS_KEY_TG_GREET, (int)i);
        load_str(h, k, out->notify.greetings[i], sizeof(out->notify.greetings[i]));
    }

    uint32_t u32;
    if (load_u32(h, NVS_KEY_DEBOUNCE_MS, &u32)) out->sensor.debounce_ms = u32;
    if (load_u32(h, NVS_KEY_ALERT_MS, &u32)) out->sensor.alert_interval_ms = u32;
    if (load_u32(h, NVS_KEY_LED_GPIO, &u32)) out->sensor.led_gpio = (gpio_num_t)u32;

    uint8_t away;
    if (load_u8(h, NVS_KEY_AWAY, &away)) out->state.away_mode = (away != 0);
    if (load_u8(h, NVS_KEY_QUIET_EN, &away)) out->state.quiet_enabled = (away != 0);
    if (load_u8(h, NVS_KEY_QUIET_START, &n)) out->state.quiet_start_h = n;
    if (load_u8(h, NVS_KEY_QUIET_END, &n)) out->state.quiet_end_h = n;

    load_str(h, NVS_KEY_MQTT_URI, out->notify.mqtt_uri, sizeof(out->notify.mqtt_uri));
    load_str(h, NVS_KEY_MQTT_TOPIC, out->notify.mqtt_topic, sizeof(out->notify.mqtt_topic));

    nvs_close(h);
    ESP_LOGI(TAG, "Config loaded from NVS");
}

void app_config_save(const app_config_t *c)
{
    if (!c) return;
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK) return;

    char k[32];
    nvs_set_str(h, NVS_KEY_SSID, c->wifi.ssid);
    nvs_set_str(h, NVS_KEY_PASS, c->wifi.password);
    nvs_set_str(h, NVS_KEY_TG_TOKEN, c->notify.token);
    nvs_set_u8(h, NVS_KEY_ZONE_COUNT, c->sensor.zone_count);

    for (int i = 0; i < c->sensor.zone_count && i < APP_CONFIG_MAX_ZONES; i++) {
        snprintf(k, sizeof(k), NVS_KEY_ZONE_GPIO, i);
        nvs_set_u32(h, k, (uint32_t)c->sensor.zones[i].gpio);
        snprintf(k, sizeof(k), NVS_KEY_ZONE_LABEL, i);
        nvs_set_str(h, k, c->sensor.zones[i].label);
    }

    nvs_set_u8(h, NVS_KEY_TG_NUM, (uint8_t)(c->notify.num_chats <= APP_CONFIG_MAX_CHATS ? c->notify.num_chats : APP_CONFIG_MAX_CHATS));
    for (size_t i = 0; i < c->notify.num_chats && i < APP_CONFIG_MAX_CHATS; i++) {
        snprintf(k, sizeof(k), NVS_KEY_TG_CHAT, (int)i);
        nvs_set_str(h, k, c->notify.chat_ids[i]);
        snprintf(k, sizeof(k), NVS_KEY_TG_GREET, (int)i);
        nvs_set_str(h, k, c->notify.greetings[i]);
    }

    nvs_set_u32(h, NVS_KEY_DEBOUNCE_MS, c->sensor.debounce_ms);
    nvs_set_u32(h, NVS_KEY_ALERT_MS, c->sensor.alert_interval_ms);
    nvs_set_u32(h, NVS_KEY_LED_GPIO, (uint32_t)c->sensor.led_gpio);
    nvs_set_u8(h, NVS_KEY_AWAY, c->state.away_mode ? 1 : 0);
    nvs_set_u8(h, NVS_KEY_QUIET_EN, c->state.quiet_enabled ? 1 : 0);
    nvs_set_u8(h, NVS_KEY_QUIET_START, c->state.quiet_start_h);
    nvs_set_u8(h, NVS_KEY_QUIET_END, c->state.quiet_end_h);
    nvs_set_str(h, NVS_KEY_MQTT_URI, c->notify.mqtt_uri);
    nvs_set_str(h, NVS_KEY_MQTT_TOPIC, c->notify.mqtt_topic);

    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "Config saved to NVS");
}

void app_config_to_wifi_mgr(const app_config_t *c, wifi_mgr_config_t *w)
{
    if (!c || !w) return;
    w->ssid = c->wifi.ssid;
    w->password = c->wifi.password;
}

void app_config_to_notify(const app_config_t *c, notify_config_t *n)
{
    if (!c || !n) return;
    memset(n, 0, sizeof(*n));
    n->telegram_bot_token = c->notify.token;
    n->num_chats = c->notify.num_chats;
    for (size_t i = 0; i < c->notify.num_chats && i < APP_CONFIG_MAX_CHATS; i++) {
        n->telegram_chat_ids[i] = c->notify.chat_ids[i];
        n->greetings[i] = c->notify.greetings[i];
    }
    n->mqtt_uri = (c->notify.mqtt_uri[0] != '\0') ? c->notify.mqtt_uri : NULL;
    n->mqtt_topic = (c->notify.mqtt_topic[0] != '\0') ? c->notify.mqtt_topic : NULL;
}

void app_config_to_door_sensor(const app_config_t *c, door_sensor_config_t *d)
{
    if (!c || !d) return;
    memset(d, 0, sizeof(*d));
    d->zone_count = c->sensor.zone_count;
    d->debounce_ms = c->sensor.debounce_ms;
    for (int i = 0; i < c->sensor.zone_count && i < DOOR_SENSOR_MAX_ZONES; i++) {
        d->zones[i].gpio = c->sensor.zones[i].gpio;
        strncpy_safe(d->zones[i].label, c->sensor.zones[i].label, sizeof(d->zones[i].label));
    }
}
