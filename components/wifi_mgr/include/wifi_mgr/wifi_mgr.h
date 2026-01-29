#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *ssid;
    const char *password;
} wifi_mgr_config_t;

esp_err_t wifi_mgr_init(const wifi_mgr_config_t *config);
esp_err_t wifi_mgr_wait_connected(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_MGR_H */
