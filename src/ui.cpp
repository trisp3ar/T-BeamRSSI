#include "ui.h"
#include "config.h"

const byte switchPin = 38;
static byte oldSwitchState = HIGH;
static int toggle = 0;

void ui_init() {
    pinMode(switchPin, INPUT_PULLUP);
}

void ui_update() {
    // simple debounce/read; no address toggling in multi-node mode
    byte switchState = digitalRead(switchPin);
    if (switchState != oldSwitchState) {
        oldSwitchState = switchState;
        delay(10);
    }
}

int ui_getToggle() { return toggle; }

// Return codes: 0 = no event, 1 = short press, 2 = long press
int ui_pollEvent() {
    static int lastState = HIGH;
    static unsigned long pressStart = 0;
    static bool pressed = false;
    const unsigned long LONG_MS = 700;

    int state = digitalRead(switchPin);
    int result = 0;

    if (state == LOW && lastState == HIGH) {
        // button just pressed
        pressStart = millis();
        pressed = true;
    }

    if (state == HIGH && lastState == LOW && pressed) {
        unsigned long dur = millis() - pressStart;
        if (dur >= LONG_MS) result = 2; else result = 1;
        pressed = false;
    }

    lastState = state;
    return result;
}
