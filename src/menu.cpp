#include "menu.h"
#include "display_wrapper.h"
#include "config.h"
#include "lora_wrapper.h"
#include "status.h"


static bool active = false;
static int menuIndex = 0;
static bool editing = false;

static const int MENU_COUNT = 5;

void menu_init() {
    active = false;
    menuIndex = 0;
}

void menu_enter() {
    active = true;
    menuIndex = 0;
    editing = false;
}

bool menu_isActive() { return active; }

static String txPowerToString(int p) {
    return String(p) + " dBm";
}

void menu_loop(int ui_event) {
    if (!active) return;

    // Process any incoming packets so link stays alive while menu is active
    int packetSize = lora_parsePacket();
    if (packetSize > 0) {
        uint16_t sender = 0;
        String payload;
        if (lora_readIncoming(packetSize, sender, payload)) {
            rssi = "RSSI:   " + String(lora_packetRssi()) + " dBm";
            SNR = "SNR:     " + String(lora_packetSnr()) + " dB";
            linked = true;
            lastLinkTime = millis();
            // print discovered peer
            char buf[8]; sprintf(buf, "0x%04X", sender);
            Serial.print("Discovered peer: "); Serial.println(buf);
        }
    }

    // Titles: Node ID, TX Power, Run RSSI Sweep, Save & Exit, Factory Reset
    String title;
    String val;
    String hint = "short=next long=ok";

    switch (menuIndex) {
        case 0: title = "Node ID"; val = config_nodeIdHex(); break;
        case 1: title = "TX Power"; val = txPowerToString(txPower); break;
        case 2: title = "Run RSSI Sweep"; val = "Press long to run"; break;
        case 3: title = "Save & Exit"; val = "Save settings"; break;
        case 4: title = "Factory Reset"; val = "Reset defaults"; break;
    }

    display_showMenu(title, val, hint);

    if (ui_event == 0) return;

    static int sweep_remaining = 0;

    if (!editing) {
        if (ui_event == 1) {
            // short press: next menu item
            menuIndex = (menuIndex + 1) % MENU_COUNT;
        } else if (ui_event == 2) {
            // long press: enter/edit/activate
            if (menuIndex == 2) {
                // Start a non-blocking RSSI sweep: send 1 ping per menu tick
                sweep_remaining = 10; // send 10 pings
            } else if (menuIndex == 3) {
                // Save & Exit
                config_save();
                config_apply();
                active = false;
            } else if (menuIndex == 4) {
                // Factory Reset
                config_factoryReset();
                active = false;
            } else if (menuIndex == 0) {
                // long press on Node ID exits without saving
                active = false;
            } else {
                editing = true;
            }
        }
    } else {
        // editing mode: short press increment, long press confirm
        // Node ID editing disabled here; user changes manually elsewhere
        if (menuIndex == 1) {
            // TX power selection
            if (ui_event == 1) {
                if (txPower == 20) txPower = 14;
                else if (txPower == 14) txPower = 10;
                else txPower = 20;
            }
            if (ui_event == 2) { editing = false; }
        }
        
    }

    // handle non-blocking sweep: send one ping per menu loop iteration
    if (sweep_remaining > 0) {
        uint16_t broadcast = 0xFFFF;
        lora_sendMessageTo(broadcast, "Ping");
        sweep_remaining--;
        String prog = "Remaining: " + String(sweep_remaining);
        display_showMenu("RSSI Sweep", prog, "short=skip");
        if (sweep_remaining == 0) {
            display_showMenu("RSSI Sweep", "Done", "short=back");
        }
    }
}
