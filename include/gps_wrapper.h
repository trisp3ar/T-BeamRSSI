#ifndef GPS_WRAPPER_H
#define GPS_WRAPPER_H

#include <Arduino.h>

void gps_init();
void gps_update();
bool gps_hasFix();
int gps_getSatellites(); // number of satellites used in the fix, 0 if none/unknown
bool gps_isConnected(); // true once any bytes have been received from the GPS module
double gps_getLatitude(); // degrees, valid only when gps_hasFix() is true
double gps_getLongitude(); // degrees, valid only when gps_hasFix() is true
bool gps_hasLastKnownPosition(); // true once a valid GPS position was stored
double gps_getLastLatitude(); // persisted last known latitude in degrees
double gps_getLastLongitude(); // persisted last known longitude in degrees
void gps_print_debug();

#endif
