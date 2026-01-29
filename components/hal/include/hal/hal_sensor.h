#ifndef HAL_SENSOR_H
#define HAL_SENSOR_H

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hal_sensor_init(gpio_num_t gpio);
bool hal_sensor_read_closed(gpio_num_t gpio);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SENSOR_H */
