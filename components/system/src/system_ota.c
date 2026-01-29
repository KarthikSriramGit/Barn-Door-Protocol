#ifdef CONFIG_DOOR_MONITOR_OTA

#include "system/system.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "system_ota";

esp_err_t system_ota_check(const char *url)
{
    if (!url || url[0] == '\0') return ESP_ERR_INVALID_ARG;

    esp_http_client_config_t http_cfg = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_https_ota_config_t ota_cfg = {
        .http_config = &http_cfg,
    };
    ESP_LOGI(TAG, "OTA from %s", url);
    esp_err_t err = esp_https_ota(&ota_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "OTA OK, restarting");
    esp_restart();
    return ESP_OK;
}

#else

#include "system/system.h"

esp_err_t system_ota_check(const char *url)
{
    (void)url;
    return ESP_ERR_NOT_SUPPORTED;
}

#endif
