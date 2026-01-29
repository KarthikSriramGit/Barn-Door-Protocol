#include "app_state.h"
#include "sensor/door_sensor.h"
#include <string.h>

static bool s_away = true;
static door_sensor_state_t s_last_sensor;
static int64_t s_last_alert_time;

void app_state_init(void)
{
    s_away = true;
    memset(&s_last_sensor, 0, sizeof(s_last_sensor));
    s_last_alert_time = 0;
}

bool app_state_is_away(void)
{
    return s_away;
}

void app_state_set_away(bool away)
{
    s_away = away;
}

void app_state_set_last_sensor(const door_sensor_state_t *st)
{
    if (st) memcpy(&s_last_sensor, st, sizeof(s_last_sensor));
}

void app_state_get_last_sensor(door_sensor_state_t *out)
{
    if (out) memcpy(out, &s_last_sensor, sizeof(s_last_sensor));
}

void app_state_set_last_alert_time(int64_t t)
{
    s_last_alert_time = t;
}

int64_t app_state_get_last_alert_time(void)
{
    return s_last_alert_time;
}

bool app_state_any_open(void)
{
    return door_sensor_any_open(&s_last_sensor);
}
