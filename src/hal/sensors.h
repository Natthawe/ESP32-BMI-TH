#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "src/core/states.h"

bool checkHX711Startup(uint32_t timeoutMs);
float readDistanceRawCm();
RcwlStartupCheckResult checkRCWL1655Startup(float* medianOut);
void waitForSensorRetry();
void runStartupSensorCheckAndTare();

float readDistanceCm();
void sortFloatArray(float arr[], uint8_t count);
float readDistanceMedianCm(uint8_t samples);
void serviceDistanceSensor(uint32_t now);
float captureFinalDistanceCm();
float filterDistanceJump(float newDistanceCm);
bool isPersonDetected();
float readDistanceForPersonLeaveCm();
bool isPersonLeave();
float readWeightKg();
float calculateHeightCm(float distanceCm);

#endif
