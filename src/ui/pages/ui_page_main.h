#ifndef UI_PAGE_MAIN_H
#define UI_PAGE_MAIN_H

#include <Arduino.h>

void drawPageStarting(const String& line1, const String& line2);
void drawBasicPage(const String& title, const String& message, uint16_t titleColor, const String& footer);

void drawPageSensorCheck();
void drawPageSensorError();
void drawPageMainMenu();

#endif
