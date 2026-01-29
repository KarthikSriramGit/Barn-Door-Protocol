#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "app_config.h"
#include "app_state.h"
#include "app_scheduler.h"
#include "hal/hal_led.h"
#include "sensor/door_sensor.h"
#ifdef CONFIG_DOOR_MONITOR_BUZZER
#include "hal/hal_buzzer.h"
#endif
#include "wifi_mgr/wifi_mgr.h"
#include "notify/notify.h"
#include "system/system.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    app_config_t cfg;
    app_config_load(&cfg);

    wifi_mgr_config_t wc;
    app_config_to_wifi_mgr(&cfg, &wc);
    ESP_ERROR_CHECK(wifi_mgr_init(&wc));
    ESP_ERROR_CHECK(wifi_mgr_wait_connected(60000));

    app_scheduler_config_t sched_cfg = {
        .quiet_start_h = cfg.state.quiet_start_h,
        .quiet_end_h = cfg.state.quiet_end_h,
        .quiet_enabled = cfg.state.quiet_enabled,
    };
    app_scheduler_init(&sched_cfg);
    app_state_init();
    app_state_set_away(cfg.state.away_mode);

    door_sensor_config_t sc;
    app_config_to_door_sensor(&cfg, &sc);
    ESP_ERROR_CHECK(door_sensor_init(&sc));

    ESP_ERROR_CHECK(hal_led_init(cfg.sensor.led_gpio));

#ifdef CONFIG_DOOR_MONITOR_BUZZER
    if (hal_buzzer_init(CONFIG_DOOR_MONITOR_BUZZER_GPIO) != ESP_OK) {
        ESP_LOGW(TAG, "Buzzer init failed");
    }
#endif

    notify_config_t nc;
    app_config_to_notify(&cfg, &nc);
    notify_init(&nc);

#ifdef CONFIG_DOOR_MONITOR_TELEGRAM_POLL
    notify_set_away_callback(&app_state_set_away);
    notify_start_poll_task();
#endif

    esp_err_t wdt_ok = system_wdt_init();
    if (wdt_ok != ESP_OK) {
        ESP_LOGW(TAG, "WDT init failed: %s", esp_err_to_name(wdt_ok));
    }

#ifdef CONFIG_DOOR_MONITOR_OTA
    if (strlen(CONFIG_DOOR_MONITOR_OTA_URL) > 0) {
        esp_err_t ota_err = system_ota_check(CONFIG_DOOR_MONITOR_OTA_URL);
        if (ota_err != ESP_OK && ota_err != ESP_ERR_NOT_SUPPORTED) {
            ESP_LOGW(TAG, "OTA check failed: %s", esp_err_to_name(ota_err));
        }
    }
#endif

#ifdef CONFIG_DOOR_MONITOR_HEARTBEAT
    if (system_heartbeat_start(CONFIG_DOOR_MONITOR_HEARTBEAT_MS) != ESP_OK) {
        ESP_LOGW(TAG, "Heartbeat start failed");
    }
#endif

    door_sensor_state_t st;
    int64_t last_alert = 0;

    while (1) {
        door_sensor_poll(&st);
        app_state_set_last_sensor(&st);

        bool any_open = door_sensor_any_open(&st);
        hal_led_set(cfg.sensor.led_gpio, any_open);

#ifdef CONFIG_DOOR_MONITOR_BUZZER
        bool buzz = any_open && app_state_is_away() && app_scheduler_should_alert();
        hal_buzzer_set(CONFIG_DOOR_MONITOR_BUZZER_GPIO, buzz);
#endif

        int64_t now = esp_timer_get_time() / 1000;

        if (st.changed) {
            ESP_LOGW(TAG, "Door/Window is %s", any_open ? "OPEN" : "CLOSED");
            if (any_open && app_state_is_away() && app_scheduler_should_alert()) {
                notify_send_open();
                last_alert = now;
                app_state_set_last_alert_time(now);
            }
            if (!any_open && app_state_is_away() && app_scheduler_should_alert()) {
                notify_send_closed();
            }
        }

        if (any_open && app_state_is_away() && app_scheduler_should_alert() &&
            (now - last_alert >= (int64_t)cfg.sensor.alert_interval_ms)) {
            notify_send_open();
            last_alert = now;
            app_state_set_last_alert_time(now);
        }

        if (wdt_ok == ESP_OK) {
            system_wdt_feed();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
