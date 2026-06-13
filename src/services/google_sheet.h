#ifndef SERVICES_GOOGLE_SHEET_H
#define SERVICES_GOOGLE_SHEET_H

#include <Arduino.h>

bool sendToGoogleSheet(float weightKg, float distanceCm, float heightCm, float bmi, const char* statusText);

#endif // SERVICES_GOOGLE_SHEET_H