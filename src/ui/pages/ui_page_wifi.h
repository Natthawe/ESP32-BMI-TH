#ifndef UI_PAGE_WIFI_H
#define UI_PAGE_WIFI_H

#include <Arduino.h>

void drawPageWifiFailed();
void drawPageWifiSettings();
void drawPageWifiConnectingUi();
void drawWifiConnectingStatusLine(const String& text);
void drawPageWifiSuccessUi();
void drawPageWifiFailedUi();

#endif
