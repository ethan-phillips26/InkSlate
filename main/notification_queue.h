#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t  category_id;
    uint8_t  event_id;
    char app_id[32];
    char title[64];
    char subtitle[64];
    char message[128];
} notification_t;

extern QueueHandle_t g_notification_queue;

void notification_queue_init(void);

#ifdef __cplusplus
}
#endif