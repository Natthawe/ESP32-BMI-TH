#ifndef THAI_TEXT_H
#define THAI_TEXT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

uint16_t thaiReadCodepoint(const String& s, int& i);
String thaiUtf8FromCodepoint(uint16_t cp);

bool thaiIsUpperMark(uint16_t cp);
bool thaiIsLowerMark(uint16_t cp);
bool thaiIsToneMark(uint16_t cp);
bool thaiIsCombiningNoAdvance(uint16_t cp);
bool thaiIsSaraAm(uint16_t cp);

int thaiTextWidthSimple(TFT_eSPI& tft, const String& text);
void thaiDrawTextSimple(TFT_eSPI& tft, const String& text, int x, int y, uint16_t color, uint16_t bg);

#endif
