#include "display_wrapper.h"
#include "SSD1306.h"
#include "config.h"
#include "peers.h"
#include "battery.h"

SSD1306 display(0x3c, 21, 22);

void display_init() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void display_showStatus(bool linked, const String &rssi, const String &localIdHex) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    if (linked) {
        display.drawString(0, 0, "LoRa linked");
    } else {
        display.drawString(0, 0, "Link failed");
    }
    display.drawString(0, 16, rssi);
    display.drawString(0, 32, "Local: " + localIdHex);
    display.display();
}

void display_showMenu(const String &title, const String &value, const String &hint, bool titleInverted, bool valueInverted) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    // Title area (y=0..15)
    display.setFont(ArialMT_Plain_10);
    if (titleInverted) {
        display.setColor(WHITE);
        display.fillRect(0, 0, 128, 16);
        display.setColor(BLACK);
        display.drawString(0, 0, title);
        display.setColor(WHITE);
    } else {
        display.setColor(WHITE);
        display.drawString(0, 0, title);
    }

    // Value area (y ~18..35)
    display.setFont(ArialMT_Plain_16);
    if (valueInverted) {
        display.setColor(WHITE);
        display.fillRect(0, 16, 128, 26);
        display.setColor(BLACK);
        display.drawString(0, 18, value);
        display.setColor(WHITE);
    } else {
        display.setColor(WHITE);
        display.drawString(0, 18, value);
    }

    // Hint at bottom
    display.setFont(ArialMT_Plain_10);
    display.setColor(WHITE);
    display.drawString(0, 50, hint);
    display.display();
}

void display_showMain(int page) {
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    // top-left: local node id
    display.drawString(0, 0, config_nodeIdHex());
    // top-right: battery SOC (percent)
    int soc = battery_get_soc();
    char sbuf[8]; sprintf(sbuf, "%d%%", soc);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(127, 0, String(sbuf));
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    // list peers
    PeerEntry entries[PEERS_PER_PAGE];
    int got = peers_get_page(page, entries, PEERS_PER_PAGE);
    for (int i = 0; i < got; ++i) {
        int y = 12 + i * 10;
        char buf[64];
        sprintf(buf, "0x%04X %ddBm", entries[i].id, entries[i].rssi);
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
