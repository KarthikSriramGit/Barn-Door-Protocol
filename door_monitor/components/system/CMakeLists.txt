set(SYSTEM_SRCS "src/system_wdt.c" "src/system_heartbeat.c")
set(SYSTEM_REQS "esp_timer" "esp_common" "esp_system")
if(CONFIG_DOOR_MONITOR_OTA)
    list(APPEND SYSTEM_SRCS "src/system_ota.c")
    list(APPEND SYSTEM_REQS "esp_https_ota" "esp_crt_bundle")
endif()

idf_component_register(
    SRCS ${SYSTEM_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${SYSTEM_REQS}
)
