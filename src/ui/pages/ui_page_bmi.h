#ifndef UI_PAGE_BMI_H
#define UI_PAGE_BMI_H

#include <Arduino.h>

void drawResultValues(const String& title, const String& message, float weightKg, float heightCm, float bmi, uint16_t titleColor);

void drawPageReady();
void drawPageChecking();
void drawPageRemoveObject();
void drawPageDetected();
void drawPageMeasuring(float weightKg, float heightCm, float bmi);
void drawPageCalculating();
void drawPageSending();
void drawPageResult();
void drawPagePleaseLeave();
void drawTftScreen(bool force);
void drawTareCountdown(uint8_t secondsLeft);

#endif
