set(NOTIFY_SRCS "src/notify_telegram.c" "src/notify_mqtt.c" "src/notify_telegram_poll.c")
set(NOTIFY_REQS "esp_http_client" "esp_crt_bundle")
if(CONFIG_DOOR_MONITOR_MQTT)
    list(APPEND NOTIFY_REQS "mqtt")
endif()
if(CONFIG_DOOR_MONITOR_TELEGRAM_POLL)
    list(APPEND NOTIFY_REQS "json")
endif()

idf_component_register(
    SRCS ${NOTIFY_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${NOTIFY_REQS}
)
