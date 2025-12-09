// Copyright 2017-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "ble_ancs.h"
#include <inttypes.h>
#include "notification_queue.h"


#define BLE_ANCS_TAG  "BLE_ANCS"

/*
| EventID(1 Byte) | EventFlags(1 Byte) | CategoryID(1 Byte) | CategoryCount(1 Byte) | NotificationUID(4 Bytes) |

A GATT notification delivered through the Notification Source characteristic contains the following information:
* EventID: This field informs the accessory whether the given iOS notification was added, modified, or removed. The enumerated values for this field are defined
            in EventID Values.
* EventFlags: A bitmask whose set bits inform an NC of specificities with the iOS notification. For example, if an iOS notification is considered “important”,
              the NC may want to display a more aggressive user interface (UI) to make sure the user is properly alerted. The enumerated bits for this field
              are defined in EventFlags.
* CategoryID: A numerical value providing a category in which the iOS notification can be classified. The NP will make a best effort to provide an accurate category
              for each iOS notification. The enumerated values for this field are defined in CategoryID Values.
* CategoryCount: The current number of active iOS notifications in the given category. For example, if two unread emails are sitting in a user’s email inbox, and a new
                 email is pushed to the user’s iOS device, the value of CategoryCount is 3.
* NotificationUID: A 32-bit numerical value that is the unique identifier (UID) for the iOS notification. This value can be used as a handle in commands sent to the
                   Control Point characteristic to interact with the iOS notification.
*/

char *EventID_to_String(uint8_t EventID)
{
    char *str = NULL;
    switch (EventID)
    {
        case EventIDNotificationAdded:
            str = "New message";
            break;
        case EventIDNotificationModified:
            str = "Modified message";
            break;
        case EventIDNotificationRemoved:
            str = "Removed message";
            break;
        default:
            str = "unknown EventID";
            break;
    }
    return str;
}

char *CategoryID_to_String(uint8_t CategoryID)
{
    char *Cidstr = NULL;
    switch(CategoryID) {
        case CategoryIDOther:
            Cidstr = "Other";
            break;
        case CategoryIDIncomingCall:
            Cidstr = "IncomingCall";
            break;
        case CategoryIDMissedCall:
            Cidstr = "MissedCall";
            break;
        case CategoryIDVoicemail:
            Cidstr = "Voicemail";
            break;
        case CategoryIDSocial:
            Cidstr = "Social";
            break;
        case CategoryIDSchedule:
            Cidstr = "Schedule";
            break;
        case CategoryIDEmail:
            Cidstr = "Email";
            break;
        case CategoryIDNews:
            Cidstr = "News";
            break;
        case CategoryIDHealthAndFitness:
            Cidstr = "HealthAndFitness";
            break;
        case CategoryIDBusinessAndFinance:
            Cidstr = "BusinessAndFinance";
            break;
        case CategoryIDLocation:
            Cidstr = "Location";
            break;
        case CategoryIDEntertainment:
            Cidstr = "Entertainment";
            break;
        default:
            Cidstr = "Unknown CategoryID";
            break;
    }
    return Cidstr;
}

/*
| EventID(1 Byte) | EventFlags(1 Byte) | CategoryID(1 Byte) | CategoryCount(1 Byte) | NotificationUID(4 Bytes) |
*/

void esp_receive_apple_notification_source(uint8_t *message, uint16_t message_len)
{
    if (!message || message_len < 5) {
        return;
    }

    uint8_t EventID    = message[0];
    char    *EventIDS  = EventID_to_String(EventID);
    uint8_t EventFlags = message[1];
    uint8_t CategoryID = message[2];
    char    *Cidstr    = CategoryID_to_String(CategoryID);
    uint8_t CategoryCount = message[3];
    uint32_t NotificationUID = (message[4]) | (message[5]<< 8) | (message[6]<< 16) | (message[7] << 24);
    ESP_LOGI(BLE_ANCS_TAG,
         "EventID:%s EventFlags:0x%02x CategoryID:%s CategoryCount:%d NotificationUID:%" PRIu32,
         EventIDS, EventFlags, Cidstr, CategoryCount, (uint32_t)NotificationUID);

    }
void esp_receive_apple_data_source(uint8_t *message, uint16_t message_len)
{
    if (!message || message_len == 0) {
        return;
    }

    uint8_t Command_id = message[0];

    switch (Command_id)
    {
        case CommandIDGetNotificationAttributes:
        {
            uint32_t NotificationUID =
                (uint32_t)message[1]        |
                ((uint32_t)message[2] << 8) |
                ((uint32_t)message[3] << 16)|
                ((uint32_t)message[4] << 24);

            uint32_t remaining_attr_len = message_len - 5;
            uint8_t *attrs = &message[5];

            ESP_LOGI(BLE_ANCS_TAG,
                     "receive Notification Attributes response Command_id %d NotificationUID:%" PRIu32,
                     Command_id, NotificationUID);

            notification_t n;
            memset(&n, 0, sizeof(n));


            while (remaining_attr_len > 0) {
                uint8_t AttributeID = attrs[0];
                uint16_t len = (uint16_t)attrs[1] | ((uint16_t)attrs[2] << 8);

                if (len > (remaining_attr_len - 3)) {
                    ESP_LOGE(BLE_ANCS_TAG, "data error");
                    break;
                }

                const char *data_ptr = (const char *)&attrs[3];

                switch (AttributeID)
                {
                    case NotificationAttributeIDAppIdentifier:
                    {
                        uint16_t copy_len = len;
                        if (copy_len >= sizeof(n.app_id)) {
                            copy_len = sizeof(n.app_id) - 1;
                        }
                        memcpy(n.app_id, data_ptr, copy_len);
                        n.app_id[copy_len] = '\0';
                        esp_log_buffer_char("Identifier", data_ptr, copy_len);
                        break;
                    }
                    case NotificationAttributeIDTitle:
                    {
                        uint16_t copy_len = len;
                        if (copy_len >= sizeof(n.title)) {
                            copy_len = sizeof(n.title) - 1;
                        }
                        memcpy(n.title, data_ptr, copy_len);
                        n.title[copy_len] = '\0';
                        esp_log_buffer_char("Title", data_ptr, copy_len);
                        break;
                    }
                    case NotificationAttributeIDSubtitle:
                    {
                        uint16_t copy_len = len;
                        if (copy_len >= sizeof(n.subtitle)) {
                            copy_len = sizeof(n.subtitle) - 1;
                        }
                        memcpy(n.subtitle, data_ptr, copy_len);
                        n.subtitle[copy_len] = '\0';
                        esp_log_buffer_char("Subtitle", data_ptr, copy_len);
                        break;
                    }
                    case NotificationAttributeIDMessage:
                    {
                        uint16_t copy_len = len;
                        if (copy_len >= sizeof(n.message)) {
                            copy_len = sizeof(n.message) - 1;
                        }
                        memcpy(n.message, data_ptr, copy_len);
                        n.message[copy_len] = '\0';
                        esp_log_buffer_char("Message", data_ptr, copy_len);
                        break;
                    }
                    case NotificationAttributeIDMessageSize:
                        esp_log_buffer_char("MessageSize", data_ptr, len);
                        break;
                    case NotificationAttributeIDDate:
                        esp_log_buffer_char("Date", data_ptr, len);
                        break;
                    case NotificationAttributeIDPositiveActionLabel:
                        esp_log_buffer_hex("PActionLabel", (const uint8_t *)data_ptr, len);
                        break;
                    case NotificationAttributeIDNegativeActionLabel:
                        esp_log_buffer_hex("NActionLabel", (const uint8_t *)data_ptr, len);
                        break;
                    default:
                        esp_log_buffer_hex("unknownAttributeID", (const uint8_t *)data_ptr, len);
                        break;
                }

                attrs += (1 + 2 + len);
                remaining_attr_len -= (1 + 2 + len);
            }

            // After parsing all attributes, send it to the queue for the display task
            if (g_notification_queue != NULL) {
                if (xQueueSend(g_notification_queue, &n, 0) != pdTRUE) {
                    ESP_LOGW(BLE_ANCS_TAG, "Notification queue full, dropping notification");
                }
            }

            break;
        }

        case CommandIDGetAppAttributes:
            ESP_LOGI(BLE_ANCS_TAG, "receive APP Attributes response");
            break;

        case CommandIDPerformNotificationAction:
            ESP_LOGI(BLE_ANCS_TAG, "receive Perform Notification Action");
            break;

        default:
            ESP_LOGI(BLE_ANCS_TAG, "unknown Command ID");
            break;
    }
}


char *Errcode_to_String(uint16_t status)
{
    char *Errstr = NULL;
    switch (status) {
        case Unknown_command:
            Errstr = "Unknown_command";
            break;
        case Invalid_command:
            Errstr = "Invalid_command";
            break;
        case Invalid_parameter:
            Errstr = "Invalid_parameter";
            break;
        case Action_failed:
            Errstr = "Action_failed";
            break;
        default:
            Errstr = "unknown_failed";
            break;
    }
    return Errstr;

}
