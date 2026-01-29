#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "wifi_mgr/wifi_mgr.h"
#include "notify/notify.h"
#include "sensor/door_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_CONFIG_MAX_ZONES   8
#define APP_CONFIG_MAX_CHATS   8
#define APP_CONFIG_SSID_LEN    32
#define APP_CONFIG_PASS_LEN    64
#define APP_CONFIG_TOKEN_LEN   128
#define APP_CONFIG_CHAT_LEN    32
#define APP_CONFIG_LABEL_LEN   32

typedef struct {
    char ssid[APP_CONFIG_SSID_LEN];
    char password[APP_CONFIG_PASS_LEN];
} app_config_wifi_t;

#define APP_CONFIG_MQTT_URI_LEN  96
#define APP_CONFIG_MQTT_TOPIC_LEN 64

typedef struct {
    char token[APP_CONFIG_TOKEN_LEN];
    char chat_ids[APP_CONFIG_MAX_CHATS][APP_CONFIG_CHAT_LEN];
    char greetings[APP_CONFIG_MAX_CHATS][APP_CONFIG_LABEL_LEN];
    size_t num_chats;
    char mqtt_uri[APP_CONFIG_MQTT_URI_LEN];
    char mqtt_topic[APP_CONFIG_MQTT_TOPIC_LEN];
} app_config_notify_t;

typedef struct {
    uint8_t zone_count;
    uint32_t debounce_ms;
    uint32_t alert_interval_ms;
    gpio_num_t led_gpio;
    door_sensor_zone_t zones[APP_CONFIG_MAX_ZONES];
} app_config_sensor_t;

typedef struct {
    bool away_mode;
    uint8_t quiet_start_h;  /* 0-23, e.g. 22 = 22:00 */
    uint8_t quiet_end_h;    /* 0-23, e.g. 7 = 07:00 */
    bool quiet_enabled;
} app_config_state_t;

typedef struct {
    app_config_wifi_t wifi;
    app_config_notify_t notify;
    app_config_sensor_t sensor;
    app_config_state_t state;
} app_config_t;

void app_config_load(app_config_t *out);
void app_config_save(const app_config_t *c);
void app_config_set_defaults(app_config_t *out);
void app_config_to_wifi_mgr(const app_config_t *c, wifi_mgr_config_t *w);
void app_config_to_notify(const app_config_t *c, notify_config_t *n);
void app_config_to_door_sensor(const app_config_t *c, door_sensor_config_t *d);

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
