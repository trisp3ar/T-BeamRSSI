#include "battery.h"
#include <Arduino.h>

#ifdef ARDUINO
#include <Wire.h>
#endif

#ifdef ARDUINO
#include <axp20x.h>
static AXP20X_Class axp;
#endif

void battery_init() {
#ifdef ARDUINO
    // Initialize I2C (Wire) and AXP20X driver
    Wire.begin();
    int ret = axp.begin(Wire);
    if (ret != AXP_PASS) {
        Serial.println("AXP init failed");
        return;
    }
    // enable battery ADC measurement channel
    axp.adc1Enable(AXP202_BATT_VOL_ADC1, true);
    // enable APS (system) voltage if available
    axp.adc1Enable(AXP202_APS_VOL_ADC1, true);
    // optionally enable metering system for percentage
    axp.setMeteringSystem(true);
    // debug print initial readings
    int pct = axp.getBattPercentage();
    float v = axp.getBattVoltage();
    Serial.printf("AXP init OK, batt pct=%d, battV=%.2f\n", pct, v);
#endif
}

int battery_get_soc() {
#ifdef ARDUINO
    // Use AXP library's percentage if available
    int pct = axp.getBattPercentage();
    if (pct > 0 && pct <= 100) return pct;

    // fallback: compute from reported battery voltage
    float batt = axp.getBattVoltage();
    float battV = batt;
    // some library variants return mV as float; normalize
    if (batt > 1000.0f) battV = batt / 1000.0f;
    // map batteryV 3.0..4.2V -> 0..100
    float mapped = (battV - 3.0f) / (4.2f - 3.0f) * 100.0f;
    if (mapped < 0) mapped = 0;
    if (mapped > 100) mapped = 100;
    int out = (int)round(mapped);
    Serial.printf("AXP fallback pct=%d from V=%.3f\n", out, battV);
    return out;
#else
    return 0;
#endif
}
