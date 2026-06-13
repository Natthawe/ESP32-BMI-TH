#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#include "../../core/config.h"
#include "../../core/states.h"
#include "ui_common.h"

void drawPageKeyboard();
void serviceKeyboardPasswordReveal();
String keyboardHitKey(uint16_t x, uint16_t y);
void handleKeyboardKey(const String& key);

#endif