#include "hal/hal_sensor.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "hal_sensor";

esp_err_t hal_sensor_init(gpio_num_t gpio)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Sensor GPIO %d init", gpio);
    }
    return err;
}

bool hal_sensor_read_closed(gpio_num_t gpio)
{
    return gpio_get_level(gpio) == 0;
}
