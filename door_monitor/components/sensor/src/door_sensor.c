#include "sensor/door_sensor.h"
#include "hal/hal_sensor.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "door_sensor";

static door_sensor_config_t s_config;
static door_sensor_state_t s_state;
static uint32_t s_debounce_remaining[DOOR_SENSOR_MAX_ZONES];
static bool s_last_raw[DOOR_SENSOR_MAX_ZONES];

esp_err_t door_sensor_init(const door_sensor_config_t *config)
{
    if (!config || config->zone_count == 0 || config->zone_count > DOOR_SENSOR_MAX_ZONES) {
        return ESP_ERR_INVALID_ARG;
    }
    memcpy(&s_config, config, sizeof(s_config));
    memset(&s_state, 0, sizeof(s_state));
    memset(s_debounce_remaining, 0, sizeof(s_debounce_remaining));
    for (int i = 0; i < s_config.zone_count; i++) {
        esp_err_t err = hal_sensor_init(s_config.zones[i].gpio);
        if (err != ESP_OK) return err;
        s_last_raw[i] = hal_sensor_read_closed(s_config.zones[i].gpio);
        s_state.closed[i] = s_last_raw[i];
    }
    ESP_LOGI(TAG, "Door sensor init: %u zones, debounce %lu ms", (unsigned)s_config.zone_count, (unsigned long)s_config.debounce_ms);
    return ESP_OK;
}

void door_sensor_poll(door_sensor_state_t *out)
{
    if (!out) return;
    out->changed = false;
    for (int i = 0; i < s_config.zone_count; i++) {
        bool raw = hal_sensor_read_closed(s_config.zones[i].gpio);
        if (raw != s_last_raw[i]) {
            s_last_raw[i] = raw;
            s_debounce_remaining[i] = s_config.debounce_ms;
        }
        if (s_debounce_remaining[i] > 0) {
            if (s_debounce_remaining[i] <= 10)
                s_debounce_remaining[i] = 0;
            else
                s_debounce_remaining[i] -= 10;
            if (s_debounce_remaining[i] == 0) {
                if (s_state.closed[i] != s_last_raw[i]) {
                    s_state.closed[i] = s_last_raw[i];
                    out->changed = true;
                }
            }
        }
        out->closed[i] = s_state.closed[i];
    }
}

bool door_sensor_any_open(const door_sensor_state_t *st)
{
    if (!st) return false;
    for (int i = 0; i < s_config.zone_count; i++) {
        if (!st->closed[i]) return true;
    }
    return false;
}
