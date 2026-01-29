idf_component_register(
    SRCS "src/main.c" "src/app_config.c" "src/app_scheduler.c" "src/app_state.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash hal sensor wifi_mgr notify system
)
