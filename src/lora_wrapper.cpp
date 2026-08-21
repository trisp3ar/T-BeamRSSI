#include "lora_wrapper.h"
#include <SPI.h>
#include <LoRa.h>
#include "config.h"

#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     14
#define DI0     26

void lora_init() {
    SPI.begin(SCK,MISO,MOSI,SS);
    LoRa.setPins(SS,RST,DI0);
    if (!LoRa.begin(DEFAULT_BAND)) {
        Serial.println("LoRa init failed. Check your connections.");
        while (true) {}
    }
    LoRa.setTxPower(txPower);
    LoRa.setSignalBandwidth(31.25E3);
}

void lora_sendMessageTo(uint16_t dest, const String &outgoing) {
    LoRa.beginPacket();
    // write recipient (big-endian)
    LoRa.write((uint8_t)(dest >> 8));
    LoRa.write((uint8_t)(dest & 0xFF));
    // write sender
    LoRa.write((uint8_t)(node_id >> 8));
    LoRa.write((uint8_t)(node_id & 0xFF));
    uint8_t len = (uint8_t)outgoing.length();
    LoRa.write(len);
    LoRa.print(outgoing);
    LoRa.endPacket();
}

int lora_parsePacket() { return LoRa.parsePacket(); }
int lora_packetRssi() { return LoRa.packetRssi(); }
float lora_packetSnr() { return LoRa.packetSnr(); }

bool isBroadcast(uint16_t addr) { return addr == 0xFFFF; }

bool lora_readIncoming(int packetSize, uint16_t &sender, String &payload) {
    if (packetSize == 0) return false;

    // ensure we have at least recipient(2) + sender(2) + len(1)
    if (packetSize < 5) {
        // malformed
        while (LoRa.available()) LoRa.read();
        return false;
    }

    uint16_t recipient = 0;
    recipient |= ((uint16_t)LoRa.read()) << 8;
    recipient |= ((uint16_t)LoRa.read());

    sender = 0;
    sender |= ((uint16_t)LoRa.read()) << 8;
    sender |= ((uint16_t)LoRa.read());

    uint8_t incomingLength = (uint8_t)LoRa.read();

    payload = "";
    while (LoRa.available()) {
        payload += (char)LoRa.read();
    }

    if (incomingLength != payload.length()) {
        Serial.println("Error: Message length does not match length");
        return false;
    }

    if (recipient != node_id && !isBroadcast(recipient)) {
        // not for us
        return false;
    }

    // valid incoming for us
    return true;
}

void lora_applyConfig(long band, int txPower) {
    // Reinitialize LoRa with new settings
    LoRa.end();
    delay(50);
    SPI.begin(SCK,MISO,MOSI,SS);
    LoRa.setPins(SS,RST,DI0);
    if (!LoRa.begin(band)) {
        Serial.println("LoRa re-init failed. Keeping previous settings.");
        return;
    }
    LoRa.setTxPower(txPower);
    LoRa.setSignalBandwidth(31.25E3);
}
