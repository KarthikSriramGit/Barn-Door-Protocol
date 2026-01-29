#ifndef DOOR_SENSOR_H
#define DOOR_SENSOR_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DOOR_SENSOR_MAX_ZONES 8

typedef struct {
    gpio_num_t gpio;
    char label[32];
} door_sensor_zone_t;

typedef struct {
    uint8_t zone_count;
    uint32_t debounce_ms;
    door_sensor_zone_t zones[DOOR_SENSOR_MAX_ZONES];
} door_sensor_config_t;

typedef struct {
    bool closed[DOOR_SENSOR_MAX_ZONES];
    bool changed;
} door_sensor_state_t;

esp_err_t door_sensor_init(const door_sensor_config_t *config);
void door_sensor_poll(door_sensor_state_t *out);
bool door_sensor_any_open(const door_sensor_state_t *st);

#ifdef __cplusplus
}
#endif

#endif /* DOOR_SENSOR_H */
