#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>

#include "../../core/config.h"
#include "../../core/globals.h"

// =====================================================
// Step 10.2: UI Common Header
// - ประกาศ object/ตัวแปร global ที่ ui_common.cpp ต้องใช้จาก .ino
// - ประกาศ helper UI กลางสำหรับใช้ซ้ำในหลายหน้า
// =====================================================


bool beginTftPage(const String& pageKey, uint16_t bg = TFT_BLACK);
void requestTftPageRedraw();

// ฟังก์ชันช่วยวาดภาษาไทยแบบจัดตำแหน่งสระ/วรรณยุกต์เอง
bool isThaiUpperVowel(uint16_t cp);
bool isThaiLowerVowel(uint16_t cp);
bool isThaiToneMark(uint16_t cp);
bool isThaiCombiningMark(uint16_t cp);
uint16_t readUtf8Codepoint(const String& s, int& i);
String utf8FromCodepoint(uint16_t cp);
int thaiTextWidthFixed(const String& text);
void drawThaiStringFixed(const String& text, int x, int baseY, uint8_t fontSize, uint16_t color, uint16_t bg);

// Helper วาดข้อความ / ปุ่ม / ค่า metric ที่ใช้ร่วมกันหลายหน้า
void drawCenteredText(const String& text, int y, uint8_t textSize, uint16_t color, uint16_t bg);
void drawCenteredTextBox(const String& text, int y, int h, uint8_t textSize, uint16_t color, uint16_t bg);
void drawSmallFooter(const String& text);
void drawOfflineTouchButton(const String& label);
void drawOnlineTouchButton(const String& label);
void drawUiButton(int x, int y, int w, int h, const String& label, uint16_t borderColor, uint16_t textColor);
void drawMetricLine(int y, const String& label, const String& value, uint16_t valueColor);
void updateMetricValueOnly(int y, const String& value, uint16_t valueColor);
void updateMetricValueIfChanged(int y, const String& value, String& lastValue, uint16_t valueColor);
void updateResultValuesOnly(float weightKg, float heightCm, float bmi);
void updatePleaseLeaveValuesOnly(float weightKg, float heightCm, float bmi);
void drawLeftTextBox(int x, int y, int w, int h, const String& text, uint8_t textSize, uint16_t color, uint16_t bg);

#endif
