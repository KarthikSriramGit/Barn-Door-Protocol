#ifndef SYSTEM_H
#define SYSTEM_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t system_wdt_init(void);
esp_err_t system_wdt_feed(void);
esp_err_t system_heartbeat_start(uint32_t interval_ms);
void system_heartbeat_stop(void);

esp_err_t system_ota_check(const char *url);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_H */
