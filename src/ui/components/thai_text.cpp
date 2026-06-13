#include "thai_text.h"
#include "src/core/config.h"

uint16_t thaiReadCodepoint(const String& s, int& i)
{
  uint8_t c = (uint8_t)s[i];

  if (c < 0x80)
  {
    i += 1;
    return c;
  }

  if ((c & 0xE0) == 0xC0 && (i + 1) < s.length())
  {
    uint16_t cp = ((s[i] & 0x1F) << 6) | (s[i + 1] & 0x3F);
    i += 2;
    return cp;
  }

  if ((c & 0xF0) == 0xE0 && (i + 2) < s.length())
  {
    uint16_t cp = ((s[i] & 0x0F) << 12) |
                  ((s[i + 1] & 0x3F) << 6) |
                  (s[i + 2] & 0x3F);
    i += 3;
    return cp;
  }

  i += 1;
  return c;
}



String thaiUtf8FromCodepoint(uint16_t cp)
{
  String out = "";

  if (cp < 0x80)
  {
    out += char(cp);
  }
  else if (cp < 0x800)
  {
    out += char(0xC0 | (cp >> 6));
    out += char(0x80 | (cp & 0x3F));
  }
  else
  {
    out += char(0xE0 | (cp >> 12));
    out += char(0x80 | ((cp >> 6) & 0x3F));
    out += char(0x80 | (cp & 0x3F));
  }

  return out;
}



bool thaiIsSaraAm(uint16_t cp)
{
  return cp == 0x0E33; // ำ
}



bool thaiIsUpperMark(uint16_t cp)
{
  return cp == 0x0E31 ||                    //  ั
         (cp >= 0x0E34 && cp <= 0x0E37) ||  //  ิ  ี  ึ  ื
         cp == 0x0E47 ||                    //  ็
         cp == 0x0E4D;                      //  ํ
}



bool thaiIsLowerMark(uint16_t cp)
{
  return cp >= 0x0E38 && cp <= 0x0E3A;      // ุ  ู  ฺ
}



bool thaiIsToneMark(uint16_t cp)
{
  return cp >= 0x0E48 && cp <= 0x0E4C;      // ่  ้  ๊  ๋  ์
}



bool thaiIsCombiningNoAdvance(uint16_t cp)
{
  return thaiIsUpperMark(cp) || thaiIsLowerMark(cp) || thaiIsToneMark(cp);
}



static bool lookAheadHasSaraAmBeforeNextBase(const String& text, int startIndex)
{
  for (int j = startIndex; j < text.length(); )
  {
    int oldJ = j;
    uint16_t cp = thaiReadCodepoint(text, j);

    if (thaiIsSaraAm(cp)) return true;

    if (thaiIsCombiningNoAdvance(cp)) continue;

    (void)oldJ;
    return false;
  }

  return false;
}



int thaiTextWidthSimple(TFT_eSPI& tft, const String& text)
{
  int w = 0;

  for (int i = 0; i < text.length(); )
  {
    uint16_t cp = thaiReadCodepoint(text, i);

    // mark พวกนี้จะถูกวาดแบบไม่เลื่อน cursor
    if (thaiIsCombiningNoAdvance(cp)) continue;

    String ch = thaiUtf8FromCodepoint(cp);
    int cw = tft.textWidth(ch);

    if (cp == ' ' && cw <= 0) cw = THAI_SPACE_FALLBACK_W;

    if (thaiIsSaraAm(cp))
    {
      cw += THAI_SARA_AM_ADVANCE_ADJUST_X;
      if (cw < 0) cw = 0;
    }

    w += cw;
  }

  return w;
}



void thaiDrawTextSimple(TFT_eSPI& tft, const String& text, int x, int y, uint16_t color, uint16_t bg)
{
  tft.setTextColor(color, bg);
  tft.setTextDatum(TL_DATUM);
  tft.setCursor(x, y);

  int cursorX = x;
  int cursorY = y;
  bool hasUpperOnCurrentBase = false;
  bool hasLowerOnCurrentBase = false;

  for (int i = 0; i < text.length(); )
  {
    int cpStartIndex = i;
    uint16_t cp = thaiReadCodepoint(text, i);
    String ch = thaiUtf8FromCodepoint(cp);

    if (thaiIsUpperMark(cp))
    {
      // สระบน: วาดที่ cursor ปัจจุบัน แล้วคืน cursor ไม่ให้เดินหน้า
      tft.setCursor(cursorX + THAI_UPPER_X_OFFSET, cursorY + THAI_UPPER_Y_OFFSET);
      tft.print(ch);
      tft.setCursor(cursorX, cursorY);
      hasUpperOnCurrentBase = true;
      continue;
    }

    if (thaiIsLowerMark(cp))
    {
      // สระล่าง: วาดที่ cursor ปัจจุบัน แล้วคืน cursor ไม่ให้เดินหน้า
      tft.setCursor(cursorX + THAI_LOWER_X_OFFSET, cursorY + THAI_LOWER_Y_OFFSET);
      tft.print(ch);
      tft.setCursor(cursorX, cursorY);
      hasLowerOnCurrentBase = true;
      continue;
    }

    if (thaiIsToneMark(cp))
    {
      // แยก tone เป็น 2 layer:
      // 1) Tone เดี่ยว เช่น แท่น / ส่วน / ไม่ / ค่า
      //    ใช้ THAI_TONE_* และควรอยู่ระดับเดียวกับสระบน
      // 2) Tone ซ้อนบนสระ เช่น ชั่ง / เครื่อง / น้ำ
      //    ใช้ THAI_STACK_TONE_* และควรสูงกว่าอีกชั้น
      bool saraAmAhead = lookAheadHasSaraAmBeforeNextBase(text, i);
      bool stackedTone = hasUpperOnCurrentBase || saraAmAhead;

      int toneX = THAI_TONE_X_OFFSET;
      int toneY = THAI_TONE_Y_OFFSET;

      if (stackedTone)
      {
        toneX = THAI_STACK_TONE_X_OFFSET;
        toneY = THAI_STACK_TONE_Y_OFFSET;
      }
      else if (hasLowerOnCurrentBase)
      {
        // เช่น ปุ่ม / ทุ่ม: มีสระล่างอยู่ใต้พยัญชนะ
        // วรรณยุกต์ยังอยู่ layer เดี่ยว แต่ควรเยื้อง X หลบตัวฐานเล็กน้อย 
        // เช่น ป หากไม่ขยับไม้เอก จะทับกับหาง ป
        toneX = THAI_TONE_AFTER_LOWER_X_OFFSET;
        toneY = THAI_TONE_AFTER_LOWER_Y_OFFSET;
      }

      tft.setCursor(cursorX + toneX, cursorY + toneY);
      tft.print(ch);
      tft.setCursor(cursorX, cursorY);
      continue;
    }

    // ปรับระยะเพื่อแก้ช่องไฟหลัง ำ
    if (thaiIsSaraAm(cp))
    {
      tft.setCursor(cursorX, cursorY);
      tft.print(ch);
      cursorX = tft.getCursorX() + THAI_SARA_AM_ADVANCE_ADJUST_X;
      tft.setCursor(cursorX, cursorY);
      hasUpperOnCurrentBase = false;
      hasLowerOnCurrentBase = false;
      continue;
    }

    // ตัวปกติ / พยัญชนะ / สระหน้า / สระอา / อังกฤษ / ตัวเลข
    tft.setCursor(cursorX, cursorY);
    tft.print(ch);
    cursorX = tft.getCursorX();
    cursorY = tft.getCursorY();
    hasUpperOnCurrentBase = false;
    hasLowerOnCurrentBase = false;

    (void)cpStartIndex;
  }

  tft.setCursor(cursorX, cursorY);
}
