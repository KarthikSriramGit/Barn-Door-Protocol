#ifndef NOTIFY_H
#define NOTIFY_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *telegram_bot_token;
    const char *telegram_chat_ids[8];
    const char *greetings[8];
    size_t num_chats;
    const char *mqtt_uri;
    const char *mqtt_topic;
} notify_config_t;

void notify_init(const notify_config_t *config);
void notify_send_open(void);
void notify_send_closed(void);
void notify_send_alert(bool is_open);

void notify_mqtt_init(const notify_config_t *config);
void notify_mqtt_publish(bool is_open);

const char *notify_get_bot_token(void);
void notify_set_away_callback(void (*cb)(bool away));
void notify_start_poll_task(void);

#ifdef __cplusplus
}
#endif

#endif /* NOTIFY_H */
