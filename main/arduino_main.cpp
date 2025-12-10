#include "Arduino.h"
#include <SPI.h>
#include <GxEPD2_BW.h>
#include "notification_queue.h"

#define EPD_BUSY 4     // BUSY  -> GPIO4
#define EPD_RST  16    // RST   -> GPIO16
#define EPD_DC   17    // DC    -> GPIO17
#define EPD_CS   5     // CS    -> GPIO5

#define EPD_SCK  18    // CLK   -> GPIO18
#define EPD_MOSI 11    // DIN   -> GPIO11
#define EPD_MISO 13

#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

using EPD_DRIVER = GxEPD2_397_GDEM0397T81;
GxEPD2_BW<EPD_DRIVER, EPD_DRIVER::HEIGHT> display(
    EPD_DRIVER(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

extern "C" void ble_ancs_start(void);

int notificationX = 20;
int notificationY = 70;
const int NOTIF_LINE_HEIGHT = 30;
const int NOTIF_BOX_HEIGHT = 30;

void drawCurrentTime() {
    display.setFont(&FreeSans12pt7b);
    display.setCursor(660, display.height() - 40);
    display.print("12:23 PM");
}

void setup() {
    Serial.begin(115200);

    // Use custom SPI pins from S3
    SPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI);

    display.init(115200, true, 2, false);
    display.setRotation(2);
    display.setFullWindow();

    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Top bar
        display.fillRect(0, 0, display.width(), 40, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(10, 28);
        display.print("Notifications:");

        display.setTextColor(GxEPD_BLACK);

        drawCurrentTime();

        // Timer
        display.setCursor(20, display.height() - 40);
        display.print("Timer: 20 min");
        
    } while (display.nextPage());

    Serial.println("Display draw done.");
}

void loop() {
    if (g_notification_queue != nullptr) {
        notification_t n;

        // Removes notification from queue.
        if (xQueueReceive(g_notification_queue, &n, 0) == pdTRUE) {
            String notification = String(n.title) + ": " + String(n.message);

                int16_t windowX = 0;
                int16_t windowY = notificationY - 25;
                int16_t windowW = display.width();
                int16_t windowH = NOTIF_BOX_HEIGHT;

                // Check if we need to reset to top of screen
                if (windowY + windowH > display.height() - 60) {
                    windowY = 40;
                    notificationY = windowY + 25;
                }

                windowH = display.height() - windowY - 60;
                
                display.setPartialWindow(windowX, windowY, windowW, windowH);
                display.firstPage();
            do {
                display.setTextColor(GxEPD_BLACK);
                display.setFont(&FreeSansBold12pt7b);
                display.setCursor(notificationX, notificationY);
                notificationY += NOTIF_LINE_HEIGHT;
                display.print(notification);
                
            } while (display.nextPage());
            notificationY += NOTIF_LINE_HEIGHT;
            
        }
    delay(1000);
}
}

extern "C" void ble_ancs_start(void);

static void ble_task(void *pvParameters) {
    ble_ancs_start();
    vTaskDelete(NULL);
}

extern "C" void app_main(void) {
    initArduino();
    
    notification_queue_init();
    

    // Start BLE ANCS in its own task
    xTaskCreatePinnedToCore(
        ble_task,
        "ble_task",
        8192,
        NULL,
        5,
        NULL,
        ARDUINO_RUNNING_CORE
    );

    // FIX LATER: Clear notification queue at startup
    delay(10000);
    xQueueReset(g_notification_queue);

    setup();
    while (true) {
        loop();
        vTaskDelay(1);
    }
}

void clearFullScreen() {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}



