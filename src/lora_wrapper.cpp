#include "lora_wrapper.h"
#include <SPI.h>
#include <LoRa.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <mbedtls/aes.h>
#include <esp_system.h>
#include "config.h"
#include "gps_wrapper.h"

#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     14
#define DI0     26

static const char *LORA_KEY_FILE = "/LoRa-Key.json";
static const char *DEFAULT_AES256_KEY_HEX = "9f4c2a7e3b1d8c50a6e9f2147bd83c11e52a9d70c4b6f1382d7e5a91bc03f6d4";
static const uint8_t ENCRYPTION_VERSION = 1;
static uint8_t loraAesKey[32] = {0};
static bool loraAesReady = false;
static bool loraUseEncryption = true;

static int lora_hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static bool lora_hexToBytes(const char *hex, uint8_t *out, size_t outLen) {
    if (!hex || !out) return false;
    for (size_t i = 0; i < outLen; ++i) {
        int hi = lora_hexNibble(hex[i * 2]);
        int lo = lora_hexNibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return hex[outLen * 2] == '\0';
}

static bool lora_loadAesKeyFromJson(const String &jsonText) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonText);
    if (err) return false;

    const char *keyHex = doc["aes256_key_hex"] | doc["key_hex"] | doc["key"] | nullptr;
    if (!keyHex) return false;
    return lora_hexToBytes(keyHex, loraAesKey, sizeof(loraAesKey));
}

static void lora_initSecurity() {
    loraAesReady = false;

    if (SPIFFS.begin(false)) {
        File keyFile = SPIFFS.open(LORA_KEY_FILE, "r");
        if (keyFile) {
            String jsonText = keyFile.readString();
            keyFile.close();
            if (lora_loadAesKeyFromJson(jsonText)) {
                loraAesReady = true;
                Serial.println("LoRa AES key loaded from /LoRa-Key.json");
                return;
            }
            Serial.println("LoRa key file invalid, falling back to built-in key");
        } else {
            Serial.println("LoRa key file not found, using built-in key");
        }
    } else {
        Serial.println("SPIFFS mount failed, using built-in key");
    }

    if (lora_hexToBytes(DEFAULT_AES256_KEY_HEX, loraAesKey, sizeof(loraAesKey))) {
        loraAesReady = true;
    }
}

static bool lora_encryptPayload(const String &plain, uint8_t *out, size_t &outLen) {
    outLen = 0;
    if (!loraAesReady) return false;

    const size_t plainLen = (size_t)plain.length();
    const size_t paddedLen = ((plainLen / 16) + 1) * 16;
    const size_t totalLen = 1 + 16 + paddedLen;
    if (totalLen > 255) {
        Serial.println("LoRa payload too large for encrypted packet");
        return false;
    }

    uint8_t *padded = (uint8_t *)malloc(paddedLen);
    uint8_t *cipher = (uint8_t *)malloc(paddedLen);
    if (!padded || !cipher) {
        free(padded);
        free(cipher);
        return false;
    }

    memcpy(padded, plain.c_str(), plainLen);
    const uint8_t pad = (uint8_t)(paddedLen - plainLen);
    memset(padded + plainLen, pad, pad);

    uint8_t iv[16];
    uint8_t ivWork[16];
    for (size_t i = 0; i < sizeof(iv); ++i) {
        iv[i] = (uint8_t)(esp_random() & 0xFF);
    }
    memcpy(ivWork, iv, sizeof(iv));

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    int rc = mbedtls_aes_setkey_enc(&aes, loraAesKey, 256);
    if (rc == 0) {
        rc = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLen, ivWork, padded, cipher);
    }
    mbedtls_aes_free(&aes);

    if (rc != 0) {
        free(padded);
        free(cipher);
        return false;
    }

    out[0] = ENCRYPTION_VERSION;
    memcpy(out + 1, iv, sizeof(iv));
    memcpy(out + 17, cipher, paddedLen);
    outLen = totalLen;

    free(padded);
    free(cipher);
    return true;
}

static bool lora_decryptPayload(const uint8_t *in, size_t inLen, String &plain) {
    if (!loraAesReady || !in) return false;
    if (inLen < 17 || in[0] != ENCRYPTION_VERSION) return false;

    const size_t cipherLen = inLen - 17;
    if (cipherLen == 0 || (cipherLen % 16) != 0) return false;

    uint8_t *decrypted = (uint8_t *)malloc(cipherLen);
    if (!decrypted) return false;

    uint8_t ivWork[16];
    memcpy(ivWork, in + 1, sizeof(ivWork));

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    int rc = mbedtls_aes_setkey_dec(&aes, loraAesKey, 256);
    if (rc == 0) {
        rc = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, cipherLen, ivWork, in + 17, decrypted);
    }
    mbedtls_aes_free(&aes);

    if (rc != 0) {
        free(decrypted);
        return false;
    }

    uint8_t pad = decrypted[cipherLen - 1];
    if (pad == 0 || pad > 16 || pad > cipherLen) {
        free(decrypted);
        return false;
    }
    for (size_t i = 0; i < pad; ++i) {
        if (decrypted[cipherLen - 1 - i] != pad) {
            free(decrypted);
            return false;
        }
    }

    const size_t plainLen = cipherLen - pad;
    plain = "";
    plain.reserve(plainLen);
    for (size_t i = 0; i < plainLen; ++i) {
        plain += (char)decrypted[i];
    }

    free(decrypted);
    return true;
}

void lora_init() {
    lora_initSecurity();
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
    String payload = outgoing;
    if (gps_hasLastKnownPosition()) {
        payload += "|gps=";
        payload += String(gps_getLastLatitude(), 5);
        payload += ',';
        payload += String(gps_getLastLongitude(), 5);
    }

    LoRa.beginPacket();
    // write recipient (big-endian)
    LoRa.write((uint8_t)(dest >> 8));
    LoRa.write((uint8_t)(dest & 0xFF));
    // write sender
    LoRa.write((uint8_t)(node_id >> 8));
    LoRa.write((uint8_t)(node_id & 0xFF));

    if (loraUseEncryption) {
        uint8_t encrypted[255];
        size_t encryptedLen = 0;
        if (!lora_encryptPayload(payload, encrypted, encryptedLen)) {
            Serial.println("LoRa encryption failed. Packet not sent.");
            LoRa.endPacket();
            return;
        }
        LoRa.write((uint8_t)encryptedLen);
        LoRa.write(encrypted, encryptedLen);
    } else {
        size_t plainLen = (size_t)payload.length();
        if (plainLen == 0 || plainLen > 255) {
            Serial.println("LoRa plaintext payload length invalid. Packet not sent.");
            LoRa.endPacket();
            return;
        }
        LoRa.write((uint8_t)plainLen);
        LoRa.write((const uint8_t *)payload.c_str(), plainLen);
    }

    LoRa.endPacket();
}

int lora_parsePacket() { return LoRa.parsePacket(); }
int lora_packetRssi() { return LoRa.packetRssi(); }
float lora_packetSnr() { return LoRa.packetSnr(); }

bool isBroadcast(uint16_t addr) { return addr == 0xFFFF; }

static void lora_printReceivedGps(uint16_t sender, const String &payload) {
    int gpsStart = payload.indexOf("|gps=");
    if (gpsStart < 0) return;

    double latitude = 0.0;
    double longitude = 0.0;
    char extra = '\0';
    const char *coordinates = payload.c_str() + gpsStart + 5;
    if (sscanf(coordinates, "%lf,%lf%c", &latitude, &longitude, &extra) != 2 ||
        latitude < -90.0 || latitude > 90.0 ||
        longitude < -180.0 || longitude > 180.0) {
        Serial.printf("Received invalid GPS payload from 0x%04X\n", sender);
        return;
    }

    Serial.printf("Received GPS from 0x%04X: %.5f, %.5f\n", sender, latitude, longitude);
}

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

    if (incomingLength == 0) {
        while (LoRa.available()) LoRa.read();
        return false;
    }

    uint8_t incomingData[255] = {0};
    size_t bytesRead = 0;
    while (LoRa.available() && bytesRead < incomingLength) {
        incomingData[bytesRead++] = (uint8_t)LoRa.read();
    }
    while (LoRa.available()) LoRa.read();

    if (bytesRead != incomingLength) {
        Serial.println("Error: Payload length mismatch");
        return false;
    }

    if (packetSize != (int)(5 + incomingLength)) {
        Serial.println("Error: Packet size mismatch");
        return false;
    }

    if (recipient != node_id && !isBroadcast(recipient)) {
        // not for us
        return false;
    }

    if (loraUseEncryption) {
        if (!lora_decryptPayload(incomingData, incomingLength, payload)) {
            Serial.printf("LoRa decrypt failed for packet from 0x%04X\n", sender);
            return false;
        }
    } else {
        payload = "";
        payload.reserve(incomingLength);
        for (size_t i = 0; i < incomingLength; ++i) {
            payload += (char)incomingData[i];
        }
    }

    lora_printReceivedGps(sender, payload);

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

void lora_setEncryptionEnabled(bool enabled) {
    loraUseEncryption = enabled;
    Serial.printf("LoRa encryption: %s\n", loraUseEncryption ? "enabled" : "disabled");
}

bool lora_isEncryptionEnabled() {
    return loraUseEncryption;
}
