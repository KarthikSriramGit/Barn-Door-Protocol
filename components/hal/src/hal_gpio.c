#include "hal/hal_gpio.h"
#include "esp_log.h"

static const char *TAG = "hal_gpio";

esp_err_t hal_gpio_init(void)
{
    ESP_LOGI(TAG, "GPIO HAL init (stub)");
    return ESP_OK;
}
