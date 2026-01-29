#include "system/system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <stdint.h>

static const char *TAG = "system_heartbeat";
static esp_timer_handle_t s_timer;

static void heartbeat_cb(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "Heartbeat");
}

esp_err_t system_heartbeat_start(uint32_t interval_ms)
{
    const esp_timer_create_args_t args = {
        .callback = &heartbeat_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "heartbeat",
    };
    esp_err_t err = esp_timer_create(&args, &s_timer);
    if (err != ESP_OK) return err;
    err = esp_timer_start_periodic(s_timer, (uint64_t)interval_ms * 1000);
    if (err != ESP_OK) {
        esp_timer_delete(s_timer);
        s_timer = NULL;
        return err;
    }
    ESP_LOGI(TAG, "Heartbeat started %lu ms", (unsigned long)interval_ms);
    return ESP_OK;
}

void system_heartbeat_stop(void)
{
    if (s_timer) {
        esp_timer_stop(s_timer);
        esp_timer_delete(s_timer);
        s_timer = NULL;
    }
}
