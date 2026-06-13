#ifndef SERVICES_WIFI_MANAGER_H
#define SERVICES_WIFI_MANAGER_H

#include <Arduino.h>

bool hasWifiCredentials();
bool isOnlineMode();
String modeText();
String getWifiText();

bool connectWiFiWithUi(uint32_t timeoutMs, bool showFinalPage);
void autoConnectWiFiAtStartup();
void connectWiFiFromUi();
void connectWiFi();

#endif // SERVICES_WIFI_MANAGER_H