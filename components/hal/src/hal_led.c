#include "hal/hal_led.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "hal_led";

esp_err_t hal_led_init(gpio_num_t gpio)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "LED GPIO %d init", gpio);
    }
    return err;
}

esp_err_t hal_led_set(gpio_num_t gpio, bool on)
{
    gpio_set_level(gpio, on ? 1 : 0);
    return ESP_OK;
}
