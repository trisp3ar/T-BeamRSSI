// T-Beam, display RSSI (refactored)

#include <Arduino.h>
#include "config.h"
#include "lora_wrapper.h"
#include "display_wrapper.h"
#include "ui.h"
#include "menu.h"
#include "status.h"
#include "peers.h"
#include "battery.h"
#include "gps_wrapper.h"

long lastSendTime = 0;
int interval = 1000;
int count = 0;
int ping = 1;

// status variables (shared)
bool linked = false;
unsigned long lastLinkTime = 0;
String rssi = "";
int main_page = 0;

void setup() {
    delay(100);
    Serial.begin(9600);

    config_init();
    lora_init();
    ui_init();
    display_init();
    menu_init();
    peers_init();
    battery_init();
    gps_init();
}

void loop() {
    gps_update();
    int evt = ui_pollEvent();
    if (!menu_isActive() && evt == 2) {
        // long press enters menu; consume event so menu doesn't re-handle it
        menu_enter();
        evt = 0;
    }

    if (menu_isActive()) {
        menu_loop(evt);
    } else {
        // non-menu behavior: short press currently unused
        (void)evt;

                // non-menu behavior: short press advances main screen pages
                if (evt == 1) {
                    int pages = peers_pages();
                    if (pages == 0) pages = 1;
                    main_page = (main_page + 1) % pages;
                }
    if (millis() - lastSendTime > interval) {
        uint16_t broadcast = 0xFFFF;
        lora_sendMessageTo(broadcast, "Ping");

        lastSendTime = millis();
        interval = random(2000) + 100;
        if (ping < 255) ping++;
        else ping = 1;
    }

    if (millis() - lastLinkTime > link_time) {
        linked = false;
        lastLinkTime = millis();
    }

    int packetSize = lora_parsePacket();
    if (packetSize > 0) {
        uint16_t senderId = 0;
        String payload;
        if (lora_readIncoming(packetSize, senderId, payload)) {
            rssi = "RSSI:   " + String(lora_packetRssi()) + " dBm";
            linked = true;
            lastLinkTime = millis();
            // add peer discovery (print sender id)
            char buf[7]; sprintf(buf, "0x%04X", senderId);
            double peerLat = 0.0, peerLon = 0.0;
            bool peerHasPosition = lora_extractGps(payload, peerLat, peerLon);
            peers_add_or_update(senderId, lora_packetRssi(), peerHasPosition, peerLat, peerLon);
        }
    }
            // purge peers not seen for more than 60s
            peers_cleanup(60000);
            int pages = peers_pages();
            if (pages == 0) pages = 1;
            if (main_page >= pages) main_page = 0;
            // show main page
            display_showMain(main_page);
            // periodic battery debug print every 5s
            static unsigned long lastBatDbg = 0;
            if (millis() - lastBatDbg > 5000) {
                battery_print_debug();
                lastBatDbg = millis();
            }
            // periodic GPS location debug print every 5s
            static unsigned long lastGpsDbg = 0;
            if (millis() - lastGpsDbg > 5000) {
                gps_print_debug();
                lastGpsDbg = millis();
            }
    }
}
