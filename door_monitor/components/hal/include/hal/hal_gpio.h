#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hal_gpio_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */
