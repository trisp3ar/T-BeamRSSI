#ifndef LORA_WRAPPER_H
#define LORA_WRAPPER_H

#include <Arduino.h>

void lora_init();
void lora_sendMessageTo(uint16_t dest, const String &outgoing);
int lora_parsePacket();
int lora_packetRssi();
float lora_packetSnr();
// returns true if a valid packet addressed to this node (or broadcast) was read;
// fills sender (16-bit) and payload string
bool lora_readIncoming(int packetSize, uint16_t &sender, String &payload);

void lora_applyConfig(long band, int txPower);
void lora_setEncryptionEnabled(bool enabled);
bool lora_isEncryptionEnabled();

#endif
