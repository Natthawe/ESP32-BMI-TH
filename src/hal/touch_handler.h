#ifndef TOUCH_HANDLER_H
#define TOUCH_HANDLER_H

#include <Arduino.h>

void startBmiMeasureFromMenu();
void enterOfflineMode(const char* reason);
void handleUiTouch();

bool readTouchPoint(uint16_t* x, uint16_t* y);
bool isInsideRect(uint16_t x, uint16_t y, int rx, int ry, int rw, int rh);

bool isOfflineTouchPressed();
bool isOnlineTouchPressed();
bool waitForOfflineTouchHold(uint32_t holdMs);
bool waitForOnlineTouchHold(uint32_t holdMs);
bool waitForOfflineTouchWindow(uint32_t windowMs);
bool waitForOfflineTouchOrDelay(uint32_t waitMs);

void returnToMainMenuFromReady();
void handleReadyTouchActions();

#endif
