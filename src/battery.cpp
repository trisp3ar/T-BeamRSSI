#include "battery.h"
#include <Arduino.h>

#ifdef ARDUINO
#include <Wire.h>
#include <axp20x.h>

static AXP20X_Class axp;
// AXP192_SLAVE_ADDRESS (0x34) is already defined by axp20x.h
#endif

void battery_init() {
#ifdef ARDUINO
    Wire.begin(21, 22);
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
        Serial.println("AXP192 Begin PASS");
    } else {
        Serial.println("AXP192 Begin FAIL");
    }

    axp.setDCDC1Voltage(3300); // OLED display at full 3.3v

    axp.adc1Enable(AXP202_BATT_VOL_ADC1, true);
    axp.adc1Enable(AXP202_BATT_CUR_ADC1, true);
    axp.adc1Enable(AXP202_VBUS_VOL_ADC1, true);
    axp.adc1Enable(AXP202_VBUS_CUR_ADC1, true);

    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
#endif
}

int battery_get_soc() {
#ifdef ARDUINO
    // Reference project only reports raw voltage (VBAT = axp.getBattVoltage()/1000); map it to 0-100% for the UI.
    float battV = axp.getBattVoltage() / 1000.0f;
    const float vmin = 3.0f;
    const float vmax = 4.2f;
    float mapped = (battV - vmin) / (vmax - vmin) * 100.0f;
    if (mapped < 0.0f) mapped = 0.0f;
    if (mapped > 100.0f) mapped = 100.0f;
    return (int)round(mapped);
#else
    return 0;
#endif
}

void battery_print_debug() {
#ifdef ARDUINO
    float vbat = axp.getBattVoltage() / 1000.0f;
    Serial.printf("VBAT: %.2fV, charging=%d\n", vbat, axp.isChargeing());
#else
    Serial.println("BAT debug: not supported on host");
#endif
}