#ifndef UI_H
#define UI_H

#include <Arduino.h>

void ui_init();
void ui_update();
int ui_getToggle();
int ui_pollEvent();

#endif
