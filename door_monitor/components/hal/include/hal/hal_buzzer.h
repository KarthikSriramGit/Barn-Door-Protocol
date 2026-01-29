#ifndef HAL_BUZZER_H
#define HAL_BUZZER_H

#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t hal_buzzer_init(gpio_num_t gpio);
esp_err_t hal_buzzer_set(gpio_num_t gpio, bool on);

#ifdef __cplusplus
}
#endif

#endif /* HAL_BUZZER_H */
