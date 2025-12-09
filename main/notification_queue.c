#include "notification_queue.h"

QueueHandle_t g_notification_queue = NULL;

void notification_queue_init(void)
{
    if (g_notification_queue == NULL) {
        g_notification_queue = xQueueCreate(10, sizeof(notification_t));
    }
}

