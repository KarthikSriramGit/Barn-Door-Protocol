#ifndef APP_SCHEDULER_H
#define APP_SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t quiet_start_h;  /* 0-23 */
    uint8_t quiet_end_h;    /* 0-23 */
    bool quiet_enabled;
} app_scheduler_config_t;

void app_scheduler_init(const app_scheduler_config_t *config);
bool app_scheduler_in_quiet_hours(void);
bool app_scheduler_should_alert(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_SCHEDULER_H */
