#include "ui_common.h"
#include "thai_text.h"

// =====================================================
// Step 10.2: UI Common Helpers
// - รวม helper UI ที่ใช้ร่วมกันหลายหน้า
// - ยังไม่ย้าย logic ของแต่ละหน้าออกจาก .ino
// - ใช้ extern เพื่ออ้างอิง object/ตัวแปร global เดิมจาก .ino
// =====================================================

void drawThaiStringFixed(const String& text, int x, int baseY, uint8_t fontSize, uint16_t color, uint16_t bg)
{
  // Step 13.1:
  // เปลี่ยนมาใช้ Simple Thai Cursor Renderer
  // หลักการคือ print mark แล้วคืน cursor กลับตำแหน่งเดิม
  // เพื่อลดปัญหาสระ/วรรณยุกต์เยื้อง โดยไม่ต้องใช้ table ใหญ่
  (void)fontSize;
  thaiDrawTextSimple(tft, text, x, baseY, color, bg);
}



bool beginTftPage(const String& pageKey, uint16_t bg)
{
  if (activeTftPageKey != pageKey)
  {
    tft.fillScreen(bg);
    activeTftPageKey = pageKey;
    tftFullRedrawRequested = false;
    return true; 
  }
  tftFullRedrawRequested = false;
  return false; 
}



void requestTftPageRedraw()
{
  activeTftPageKey = "";
  tftFullRedrawRequested = true;
}



bool isThaiUpperVowel(uint16_t cp)
{
  return cp == 0x0E31 ||                    // ั
         (cp >= 0x0E34 && cp <= 0x0E37) ||  // ิ ี ึ ื
         cp == 0x0E47 ||                    // ็
         cp == 0x0E4D;                      // ํ
}



bool isThaiLowerVowel(uint16_t cp)
{
  return cp >= 0x0E38 && cp <= 0x0E3A;      // ุ ู ฺ
}



bool isThaiToneMark(uint16_t cp)
{
  return cp >= 0x0E48 && cp <= 0x0E4C;      // ่ ้ ๊ ๋ ์
}



bool isThaiCombiningMark(uint16_t cp)
{
  return isThaiUpperVowel(cp) || isThaiLowerVowel(cp) || isThaiToneMark(cp);
}



uint16_t readUtf8Codepoint(const String& s, int& i)
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



String utf8FromCodepoint(uint16_t cp)
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



int thaiTextWidthFixed(const String& text)
{
  // Step 13.1:
  // ใช้ width calculation ของ Simple Thai Cursor Renderer
  // โดย mark บน/ล่าง/วรรณยุกต์จะไม่เพิ่มความกว้าง
  return thaiTextWidthSimple(tft, text);
}




void drawCenteredText(const String& text, int y, uint8_t textSize, uint16_t color, uint16_t bg)
{
  if (textSize >= 3)       tft.loadFont("Kanit-Medium-24", LittleFS);
  else if (textSize == 2)  tft.loadFont("Kanit-Medium-20", LittleFS);
  else                     tft.loadFont("Kanit-Medium-18", LittleFS);

  int textW = thaiTextWidthFixed(text);
  int x = (480 - textW) / 2;

  // y เดิมในโปรเจคนี้ถูกใช้เหมือนตำแหน่งกลางบรรทัด จึงบวก offset เล็กน้อยให้ layout ใกล้ของเดิม
  int baseY = y + 12;
  drawThaiStringFixed(text, x, baseY, textSize, color, bg);

  tft.setTextDatum(TL_DATUM);
  tft.unloadFont();
}



void drawCenteredTextBox(const String& text, int y, int h, uint8_t textSize, uint16_t color, uint16_t bg)
{
  // เพิ่มพื้นที่ลบด้านบน/ล่าง เพื่อไม่ให้สระบนและวรรณยุกต์ถูกของเดิมทับหรือถูกลบไม่หมด
  int clearY = y - 28;
  if (clearY < 0) clearY = 0;
  int clearH = h + 56;
  if (clearY + clearH > 320) clearH = 320 - clearY;

  tft.fillRect(0, clearY, 480, clearH, bg);
  drawCenteredText(text, y, textSize, color, bg);
}



void drawSmallFooter(const String& text)
{
  tft.fillRect(0, 282, 480, 38, TFT_DARKGREY);
  tft.loadFont("Kanit-Medium-18", LittleFS);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM);

  int textW = thaiTextWidthFixed(text);
  drawThaiStringFixed(text, (480 - textW) / 2, 295, 1, TFT_WHITE, TFT_DARKGREY);
  tft.setTextDatum(TL_DATUM);
  tft.unloadFont();
}



void drawOfflineTouchButton(const String& label)
{
  tft.fillRoundRect(OFFLINE_BTN_X, OFFLINE_BTN_Y, OFFLINE_BTN_W, OFFLINE_BTN_H, 10, TFT_DARKGREY);
  tft.drawRoundRect(OFFLINE_BTN_X, OFFLINE_BTN_Y, OFFLINE_BTN_W, OFFLINE_BTN_H, 10, TFT_YELLOW);
  
  tft.loadFont("Kanit-Medium-20", LittleFS); 
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM);

  int textW = thaiTextWidthFixed(label);
  int tx = OFFLINE_BTN_X + ((OFFLINE_BTN_W - textW) / 2);
  drawThaiStringFixed(label, tx, OFFLINE_BTN_Y + 22, 2, TFT_YELLOW, TFT_DARKGREY);
  tft.setTextDatum(TL_DATUM);
  tft.unloadFont();
}



void drawOnlineTouchButton(const String& label)
{
  tft.fillRoundRect(ONLINE_BTN_X, ONLINE_BTN_Y, ONLINE_BTN_W, ONLINE_BTN_H, 10, TFT_DARKGREEN);
  tft.drawRoundRect(ONLINE_BTN_X, ONLINE_BTN_Y, ONLINE_BTN_W, ONLINE_BTN_H, 10, TFT_GREEN);
  
  tft.loadFont("Kanit-Medium-20", LittleFS);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setTextDatum(MC_DATUM);

  int textW = thaiTextWidthFixed(label);
  int tx = ONLINE_BTN_X + ((ONLINE_BTN_W - textW) / 2);
  drawThaiStringFixed(label, tx, ONLINE_BTN_Y + 22, 2, TFT_WHITE, TFT_DARKGREEN);
  tft.setTextDatum(TL_DATUM);
  tft.unloadFont();
}



// =====================================================
// UI Button Thai Vertical Balance
//
// ปัญหาที่แก้:
// - ข้อความไทยในปุ่มที่มีสระบน/วรรณยุกต์เยอะ จะดูชิดด้านบน
// - ถ้าไม่มีสระล่าง พื้นที่ด้านล่างของปุ่มจะดูโล่ง
//
// แนวทาง:
// - ตรวจข้อความในปุ่มแบบคร่าว ๆ
// - ถ้ามีสระบน/วรรณยุกต์ แต่ไม่มีสระล่าง ให้ขยับข้อความลงมากกว่า
// - ถ้ามีทั้งสระบนและสระล่าง ให้ขยับลงนิดเดียว เพราะภาพรวม balance กว่า
// =====================================================
static bool thaiTextHasUpperOrToneForButton(const String& text)
{
  int i = 0;
  while (i < text.length())
  {
    uint16_t cp = readUtf8Codepoint(text, i);

    // รวมสระอำไว้ใน upper-like ด้วย เพราะมีส่วนวงกลมด้านบน
    if (isThaiUpperVowel(cp) || isThaiToneMark(cp) || cp == 0x0E33)
    {
      return true;
    }
  }

  return false;
}



static bool thaiTextHasLowerForButton(const String& text)
{
  int i = 0;
  while (i < text.length())
  {
    uint16_t cp = readUtf8Codepoint(text, i);
    if (isThaiLowerVowel(cp))
    {
      return true;
    }
  }

  return false;
}



static int getThaiButtonAutoOffsetY(const String& label)
{
  bool hasUpperOrTone = thaiTextHasUpperOrToneForButton(label);
  bool hasLower = thaiTextHasLowerForButton(label);

  // มีสระบน/วรรณยุกต์ แต่ไม่มีสระล่าง:
  // เช่น เชื่อมต่อ WiFi, แก้ไขรหัสผ่าน, ตั้งค่า WiFi
  // ภาพรวมมักชิดบน จึงขยับลงมากกว่า
  // return มากขึ้น  = ข้อความขยับลง
  // return น้อยลง  = ข้อความขยับขึ้น
  if (hasUpperOrTone && !hasLower)
  {
    return 4;
  }

  // มีทั้งสระบน/วรรณยุกต์และสระล่าง:
  // เช่น กลับเมนูหลัก, ปุ่มกลับเมนูหลัก
  // มีส่วนล่างช่วย balance แล้ว ขยับลงเล็กน้อยพอ
  if (hasUpperOrTone && hasLower)
  {
    return 2;
  }

  // มีสระล่างอย่างเดียว หรือข้อความอังกฤษ/ตัวเลข
  return 0;
}



void drawUiButton(int x, int y, int w, int h, const String& label, uint16_t borderColor, uint16_t textColor)
{
  tft.fillRoundRect(x, y, w, h, 8, TFT_BLACK);
  tft.drawRoundRect(x, y, w, h, 8, borderColor);

  tft.loadFont("Kanit-Medium-20", LittleFS);
  tft.setTextColor(textColor, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);

  int textW = thaiTextWidthFixed(label);
  int tx = x + ((w - textW) / 2);

  // ค่าเดิมของปุ่มคือ y + ((h - 22) / 2)
  // เพิ่ม autoOffsetY เพื่อชดเชยข้อความไทยที่มีสระบน/วรรณยุกต์
  // ค่ามากขึ้น = ข้อความขยับลง
  int autoOffsetY = getThaiButtonAutoOffsetY(label);
  int ty = y + ((h - 22) / 2) + autoOffsetY;

  drawThaiStringFixed(label, tx, ty, 2, textColor, TFT_BLACK);
  tft.unloadFont();
}



void drawMetricLine(int y, const String& label, const String& value, uint16_t valueColor)
{
  tft.loadFont("Kanit-Medium-18", LittleFS);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);

  drawThaiStringFixed(label, 45, y + 10, 1, TFT_LIGHTGREY, TFT_BLACK);
  tft.unloadFont();

  tft.loadFont("Kanit-Medium-24", LittleFS);
  tft.setTextColor(valueColor, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);

  drawThaiStringFixed(value, 210, y + 6, 3, valueColor, TFT_BLACK);
  tft.unloadFont();
}



void updateMetricValueOnly(int y, const String& value, uint16_t valueColor)
{
  tft.fillRect(205, y - 24, 265, 64, TFT_BLACK); 
  tft.loadFont("Kanit-Medium-24", LittleFS); 
  tft.setTextColor(valueColor, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);

  drawThaiStringFixed(value, 210, y + 6, 3, valueColor, TFT_BLACK);
  tft.unloadFont();
}



void updateMetricValueIfChanged(int y, const String& value, String& lastValue, uint16_t valueColor)
{
  if (value == lastValue) return;
  updateMetricValueOnly(y, value, valueColor);
  lastValue = value;
}



void updateResultValuesOnly(float weightKg, float heightCm, float bmi)
{
  static String lastWeightText = "";
  static String lastHeightText = "";
  static String lastBmiText = "";

  updateMetricValueIfChanged(100, String(weightKg, 3) + " กก.", lastWeightText, TFT_WHITE);
  updateMetricValueIfChanged(150, String(heightCm, 1) + " ซม.", lastHeightText, TFT_WHITE);
  updateMetricValueIfChanged(200, String(bmi, 2), lastBmiText, TFT_WHITE);
}



void updatePleaseLeaveValuesOnly(float weightKg, float heightCm, float bmi)
{
  static String lastWeightText = "";
  static String lastHeightText = "";
  static String lastBmiText = "";

  updateMetricValueIfChanged(130, String(weightKg, 3) + " กก.", lastWeightText, TFT_WHITE);
  updateMetricValueIfChanged(175, String(heightCm, 1) + " ซม.", lastHeightText, TFT_WHITE);
  updateMetricValueIfChanged(220, String(bmi, 2), lastBmiText, TFT_WHITE);
}



void drawLeftTextBox(int x, int y, int w, int h, const String& text, uint8_t textSize, uint16_t color, uint16_t bg)
{
  tft.fillRect(x, y - 4, w, h, bg); 

  if (textSize >= 3)        tft.loadFont("Kanit-Medium-24", LittleFS);
  else if (textSize == 2)   tft.loadFont("Kanit-Medium-20", LittleFS);
  else                      tft.loadFont("Kanit-Medium-18", LittleFS);

  tft.setTextColor(color, bg);
  tft.setTextDatum(TL_DATUM);

  drawThaiStringFixed(text, x, y + 10, textSize, color, bg);
  tft.unloadFont();
}


