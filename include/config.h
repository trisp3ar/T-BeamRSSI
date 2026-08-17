#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

extern const long DEFAULT_BAND;
extern const int DEFAULT_TX_POWER;

extern uint16_t node_id;
// destinationAddress removed; use per-message destination addresses
extern int link_time;

extern int txPower;

void config_init();
void config_load();
void config_save();
void config_apply();
void config_factoryReset();
String config_nodeIdHex();

#endif
