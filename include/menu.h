#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

void menu_init();
void menu_enter();
void menu_loop(int ui_event);
bool menu_isActive();

#endif
