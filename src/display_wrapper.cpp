#include "display_wrapper.h"
#include "SSD1306.h"
#include "config.h"
#include "peers.h"

SSD1306 display(0x3c, 21, 22);

void display_init() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void display_showStatus(bool linked, const String &rssi, const String &snr, const String &localIdHex) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    if (linked) {
        display.drawString(0, 0, "LoRa linked");
    } else {
        display.drawString(0, 0, "Link failed");
    }
    display.drawString(0, 16, rssi);
    display.drawString(0, 32, snr);
    display.drawString(0, 48, "Local: " + localIdHex);
    display.display();
}

void display_showMenu(const String &title, const String &value, const String &hint) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, title);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 18, value);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 50, hint);
    display.display();
}

void display_showMain(int page) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    // top-left: local node id
    display.drawString(0, 0, config_nodeIdHex());

    // list peers
    PeerEntry entries[PEERS_PER_PAGE];
    int got = peers_get_page(page, entries, PEERS_PER_PAGE);
    for (int i = 0; i < got; ++i) {
        int y = 12 + i * 10;
        char buf[64];
        sprintf(buf, "0x%04X %ddBm %.1fdB", entries[i].id, entries[i].rssi, entries[i].snr);
        display.drawString(0, y, String(buf));
    }

    // page indicator
    int totalPages = peers_pages();
    if (totalPages < 1) totalPages = 1;
    char pbuf[16];
    sprintf(pbuf, "%d/%d", page+1, totalPages);
    display.drawString(80, 54, String(pbuf));

    display.display();
}
