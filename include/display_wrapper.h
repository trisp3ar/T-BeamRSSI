#ifndef DISPLAY_WRAPPER_H
#define DISPLAY_WRAPPER_H

#include <Arduino.h>

void display_init();
void display_showStatus(bool linked, const String &rssi, const String &snr, const String &localIdHex);
void display_showMenu(const String &title, const String &value, const String &hint, bool titleInverted=false, bool valueInverted=false);
void display_showMain(int page);

#endif
