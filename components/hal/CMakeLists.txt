set(HAL_SRCS "src/hal_gpio.c" "src/hal_sensor.c" "src/hal_led.c")
if(CONFIG_DOOR_MONITOR_BUZZER)
    list(APPEND HAL_SRCS "src/hal_buzzer.c")
endif()
idf_component_register(
    SRCS ${HAL_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES driver
)
