#ifndef HAL_LED_H
#define HAL_LED_H

#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hal_led_init(gpio_num_t gpio);
esp_err_t hal_led_set(gpio_num_t gpio, bool on);

#ifdef __cplusplus
}
#endif

#endif /* HAL_LED_H */
