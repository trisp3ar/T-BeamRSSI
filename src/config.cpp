#include "config.h"

#include <Preferences.h>
#include "lora_wrapper.h"
#include <esp_system.h>

const long DEFAULT_BAND = 869525000L; // fixed to 869.525 MHz per request
const int DEFAULT_TX_POWER = 14; // default fallback


uint16_t node_id = 0;
int link_time = 10000;

int txPower = DEFAULT_TX_POWER;
bool loraEncryptionEnabled = true;

static Preferences prefs;

void config_init() {
    prefs.begin("lora", false);
    config_load();
    config_apply();
}

void config_load() {
    // load 16-bit node id (stored as int)
    int saved = prefs.getInt("node", -1);
    if (saved < 0) {
        // generate random 16-bit id and save
        node_id = (uint16_t)(esp_random() & 0xFFFF);
        prefs.putInt("node", (int)node_id);
    } else {
        node_id = (uint16_t)saved;
    }
    txPower = prefs.getInt("txp", DEFAULT_TX_POWER);
    loraEncryptionEnabled = prefs.getBool("enc", true);
}

void config_save() {
    prefs.putInt("node", (int)node_id);
    prefs.putInt("txp", txPower);
    prefs.putBool("enc", loraEncryptionEnabled);
}

void config_apply() {
    lora_setEncryptionEnabled(loraEncryptionEnabled);
    // Apply LoRa settings (frequency fixed)
    lora_applyConfig(DEFAULT_BAND, txPower);
}

void config_factoryReset() {
    prefs.clear();
    // reset node id to newly generated random value
    node_id = (uint16_t)(esp_random() & 0xFFFF);
    txPower = DEFAULT_TX_POWER;
    loraEncryptionEnabled = true;
    config_save();
    config_apply();
}

String config_nodeIdHex() {
    char buf[7];
    sprintf(buf, "0x%04X", node_id);
    return String(buf);
}
