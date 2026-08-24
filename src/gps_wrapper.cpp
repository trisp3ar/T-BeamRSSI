#include "gps_wrapper.h"

#ifdef ARDUINO
#include <TinyGPSPlus.h>
#include <Preferences.h>

// TTGO T-Beam v1.1 GPS UART pins
// GPIO34 is input-only on the ESP32, so it must be RX, not TX.
static const int GPS_RX_PIN = 34; // ESP32 RX <- GPS TX
static const int GPS_TX_PIN = 12; // ESP32 TX -> GPS RX
static const uint32_t GPS_BAUD = 9600;
static_assert(GPS_TX_PIN < 34 || GPS_TX_PIN > 39, "GPIO34-39 are input-only and cannot be used as UART TX");

static TinyGPSPlus gps;
static Preferences gpsPrefs;
static bool hasLastKnownPosition = false;
static double lastLatitude = 0.0;
static double lastLongitude = 0.0;
#endif

void gps_init() {
#ifdef ARDUINO
    Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    gpsPrefs.begin("lora", false);
    hasLastKnownPosition = gpsPrefs.getBool("gpsValid", false);
    if (hasLastKnownPosition) {
        lastLatitude = round(gpsPrefs.getDouble("gpsLat", 0.0) * 100000.0) / 100000.0;
        lastLongitude = round(gpsPrefs.getDouble("gpsLon", 0.0) * 100000.0) / 100000.0;
        gpsPrefs.putDouble("gpsLat", lastLatitude);
        gpsPrefs.putDouble("gpsLon", lastLongitude);
    }
#endif
}

void gps_update() {
#ifdef ARDUINO
    while (Serial1.available()) {
        gps.encode(Serial1.read());
    }

    if (gps.location.isUpdated() && gps.location.isValid()) {
        lastLatitude = round(gps.location.lat() * 100000.0) / 100000.0;
        lastLongitude = round(gps.location.lng() * 100000.0) / 100000.0;
        hasLastKnownPosition = true;
        gpsPrefs.putDouble("gpsLat", lastLatitude);
        gpsPrefs.putDouble("gpsLon", lastLongitude);
        gpsPrefs.putBool("gpsValid", true);
    }
#endif
}

bool gps_hasFix() {
#ifdef ARDUINO
    return gps.location.isValid() && gps.satellites.isValid() && gps.satellites.value() > 0;
#else
    return false;
#endif
}

int gps_getSatellites() {
#ifdef ARDUINO
    return gps.satellites.isValid() ? (int)gps.satellites.value() : 0;
#else
    return 0;
#endif
}

bool gps_isConnected() {
#ifdef ARDUINO
    // TinyGPSPlus has no direct "connected" check; charsProcessed() counts bytes
    // ever fed to encode(), so it stays at its initial value if nothing is wired/powered.
    return gps.charsProcessed() > 10;
#else
    return false;
#endif
}

double gps_getLatitude() {
#ifdef ARDUINO
    return gps.location.lat();
#else
    return 0.0;
#endif
}

double gps_getLongitude() {
#ifdef ARDUINO
    return gps.location.lng();
#else
    return 0.0;
#endif
}

bool gps_hasLastKnownPosition() {
#ifdef ARDUINO
    return hasLastKnownPosition;
#else
    return false;
#endif
}

double gps_getLastLatitude() {
#ifdef ARDUINO
    return lastLatitude;
#else
    return 0.0;
#endif
}

double gps_getLastLongitude() {
#ifdef ARDUINO
    return lastLongitude;
#else
    return 0.0;
#endif
}

void gps_print_debug() {
#ifdef ARDUINO
    Serial.println(gps_isConnected() ? "GPS: connected" : "GPS: not connected (no data received)");
    if (gps.location.isValid()) {
        Serial.printf("GPS: %.6f, %.6f (sats=%d)\n", gps.location.lat(), gps.location.lng(), gps_getSatellites());
    } else if (gps_hasLastKnownPosition()) {
        Serial.printf("GPS: no fix, last known %.6f, %.6f\n", gps_getLastLatitude(), gps_getLastLongitude());
    } else {
        Serial.println("GPS: no fix");
    }
#else
    Serial.println("GPS debug: not supported on host");
#endif
}

