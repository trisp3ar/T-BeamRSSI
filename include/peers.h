#ifndef PEERS_H
#define PEERS_H

#include <Arduino.h>

#define PEERS_MAX 64
#define PEERS_PER_PAGE 4

typedef struct {
    uint16_t id;
    int rssi;
    float snr;
    unsigned long lastSeen;
    bool valid;
} PeerEntry;

void peers_init();
void peers_add_or_update(uint16_t id, int rssi, float snr);
int peers_count();
int peers_pages();
int peers_get_page(int page, PeerEntry out[], int max_out);

#endif
