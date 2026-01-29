#include "system/system.h"
#include "esp_task_wdt.h"
#include "esp_log.h"

static const char *TAG = "system_wdt";
static bool s_wdt_running;

esp_err_t system_wdt_init(void)
{
    esp_err_t err = esp_task_wdt_init(30, true);
    if (err != ESP_OK) return err;
    err = esp_task_wdt_add(NULL);
    if (err != ESP_OK) {
        esp_task_wdt_deinit();
        return err;
    }
    s_wdt_running = true;
    ESP_LOGI(TAG, "Task WDT init");
    return ESP_OK;
}

esp_err_t system_wdt_feed(void)
{
    if (!s_wdt_running) return ESP_OK;
    return esp_task_wdt_reset();
}
