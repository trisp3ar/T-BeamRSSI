#include "peers.h"
#include <Arduino.h>

static PeerEntry peers[PEERS_MAX];

void peers_init() {
    for (int i = 0; i < PEERS_MAX; ++i) peers[i].valid = false;
}

void peers_add_or_update(uint16_t id, int rssi, float snr) {
    // ignore if id == our own (optional)
    // find existing
    for (int i = 0; i < PEERS_MAX; ++i) {
        if (peers[i].valid && peers[i].id == id) {
            peers[i].rssi = rssi;
            peers[i].snr = snr;
            peers[i].lastSeen = millis();
            return;
        }
    }
    // insert into first free slot
    for (int i = 0; i < PEERS_MAX; ++i) {
        if (!peers[i].valid) {
            peers[i].valid = true;
            peers[i].id = id;
            peers[i].rssi = rssi;
            peers[i].snr = snr;
            peers[i].lastSeen = millis();
            return;
        }
    }
    // if full, replace oldest
    int oldest = 0;
    unsigned long oldestTime = ULONG_MAX;
    for (int i = 0; i < PEERS_MAX; ++i) {
        if (peers[i].lastSeen < oldestTime) { oldestTime = peers[i].lastSeen; oldest = i; }
    }
    peers[oldest].id = id;
    peers[oldest].rssi = rssi;
    peers[oldest].snr = snr;
    peers[oldest].lastSeen = millis();
    peers[oldest].valid = true;
}

int peers_count() {
    int c = 0;
    for (int i = 0; i < PEERS_MAX; ++i) if (peers[i].valid) ++c;
    return c;
}

int peers_pages() {
    int c = peers_count();
    return (c + PEERS_PER_PAGE - 1) / PEERS_PER_PAGE;
}

int peers_get_page(int page, PeerEntry out[], int max_out) {
    // collect valid peers into list (no particular order)
    PeerEntry list[PEERS_MAX];
    int idx = 0;
    for (int i = 0; i < PEERS_MAX; ++i) if (peers[i].valid) list[idx++] = peers[i];
    int start = page * PEERS_PER_PAGE;
    int copied = 0;
    for (int i = start; i < idx && copied < max_out; ++i) {
        out[copied++] = list[i];
    }
    return copied;
}
