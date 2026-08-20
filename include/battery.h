#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

void battery_init();
int battery_get_soc(); // returns 0-100 percent
void battery_print_debug();

#endif
