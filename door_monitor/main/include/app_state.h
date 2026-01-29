#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include "sensor/door_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_STATE_MAX_ZONES 8

void app_state_init(void);
bool app_state_is_away(void);
void app_state_set_away(bool away);
void app_state_set_last_sensor(const door_sensor_state_t *st);
void app_state_get_last_sensor(door_sensor_state_t *out);
void app_state_set_last_alert_time(int64_t t);
int64_t app_state_get_last_alert_time(void);
bool app_state_any_open(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATE_H */
