#include "app_scheduler.h"
#include <time.h>

static app_scheduler_config_t s_cfg;

void app_scheduler_init(const app_scheduler_config_t *config)
{
    if (config) {
        s_cfg.quiet_start_h = config->quiet_start_h;
        s_cfg.quiet_end_h = config->quiet_end_h;
        s_cfg.quiet_enabled = config->quiet_enabled;
    } else {
        s_cfg.quiet_enabled = false;
    }
}

bool app_scheduler_in_quiet_hours(void)
{
    if (!s_cfg.quiet_enabled) return false;

    time_t now = time(NULL);
    if (now < 0) return false;
    struct tm t;
    if (!localtime_r(&now, &t)) return false;
    int h = (int)t.tm_hour;
    int start = (int)s_cfg.quiet_start_h;
    int end = (int)s_cfg.quiet_end_h;

    if (start > end) {
        return h >= start || h < end;
    }
    return h >= start && h < end;
}

bool app_scheduler_should_alert(void)
{
    return !app_scheduler_in_quiet_hours();
}
