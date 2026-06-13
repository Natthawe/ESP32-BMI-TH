#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "HX711.h"
#include "states.h"

// TFT / Touch
extern TFT_eSPI tft;
extern uint16_t TOUCH_CAL_DATA[5];
extern bool tftFullRedrawRequested;
extern String activeTftPageKey;

// Online / Offline / Google Sheet
extern bool offlineMode;
extern bool uploadToGoogleSheet;
extern String lastSendStatusText;

// WiFi credentials / status text
extern String wifiSsid;
extern String wifiPassword;
extern String wifiConnectStatusText;

// HX711 / RCWL startup status
extern HX711 scale;
extern bool hx711StartupOk;
extern RcwlStartupCheckResult rcwlStartupResult;
extern float rcwlStartupDistanceCm;
extern String startupSensorMessage;

// BMI state / UI state
extern BmiState state;
extern UiPage currentUiPage;
extern UiPage previousUiPage;
extern KeyboardMode keyboardMode;
extern bool bmiRunning;

// Keyboard runtime
extern bool editingPassword;
extern String keyboardBuffer;
extern String lastKeyboardFieldDrawnText;
extern uint32_t passwordRevealUntilMs;
extern bool passwordRevealActive;

// Person / weight detection timers
extern uint32_t weightFirstStartTime;
extern bool weightFirstTimerRunning;
extern uint32_t personDetectStartTime;
extern bool personConfirmTimerRunning;
extern uint32_t personLeaveStartTime;
extern bool personLeaveTimerRunning;
extern float baselineWeightKg;

// Distance sensor runtime
extern uint32_t lastRcwlDebugPrintMs;
extern float lastGoodDistanceCm;
extern bool currentDistanceIsHeld;
extern bool distanceScanActive;
extern uint8_t distanceSampleIndex;
extern uint8_t distanceValidCount;
extern float distanceSampleBuffer[20];
extern uint32_t lastDistanceSampleTime;
extern uint32_t lastDistanceReadTime;

// Measurement runtime
extern float currentWeightKg;
extern float lastWeightKg;
extern float stableStartWeightKg;
extern float currentDistanceCm;
extern float finalWeightKg;
extern float finalDistanceCm;
extern float finalHeightCm;
extern float finalBmi;
extern uint32_t lastWeightReadTime;
extern uint32_t stableStartTime;
extern bool stableTimerRunning;
extern uint32_t lastTftUpdateTime;

// Send counters
extern int sendCount;
extern int successCount;
extern int failCount;

#endif
