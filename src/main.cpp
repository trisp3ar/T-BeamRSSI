// T-Beam, display RSSI (refactored)

#include <Arduino.h>
#include "config.h"
#include "lora_wrapper.h"
#include "display_wrapper.h"
#include "ui.h"
#include "menu.h"
#include "status.h"
#include "peers.h"

long lastSendTime = 0;
int interval = 1000;
int count = 0;
int ping = 1;

// status variables (shared)
bool linked = false;
unsigned long lastLinkTime = 0;
String rssi = "";
String SNR = "";
int main_page = 0;

void setup() {
    delay(100);
    Serial.begin(9600);
    Serial.println("Start LoRa duplex");
    Serial.print("Local node id: ");
    Serial.println(config_nodeIdHex());

    config_init();
    lora_init();
    ui_init();
    display_init();
    menu_init();
    peers_init();
}

void loop() {
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

        Serial.print("Sent Ping from node " );
        Serial.println(config_nodeIdHex());
        Serial.print("RSSI: ");
        Serial.println(lora_packetRssi());
        Serial.print("SNR: ");
        Serial.println(lora_packetSnr());

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
            SNR = "SNR:     " + String(lora_packetSnr()) + " dB";
            linked = true;
            lastLinkTime = millis();
            // add peer discovery (print sender id)
            char buf[7]; sprintf(buf, "0x%04X", senderId);
            Serial.print("Received from "); Serial.println(buf);
            Serial.print("Payload: "); Serial.println(payload);
            peers_add_or_update(senderId, lora_packetRssi(), lora_packetSnr());
        }
    }
            display_showMain(main_page);
    }
}