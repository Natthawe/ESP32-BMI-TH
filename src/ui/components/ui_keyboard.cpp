#include "ui_keyboard.h"
#include "src/services/preferences_manager.h"
#include "src/ui/pages/ui_pages.h"
#include "src/hal/touch_handler.h"
#include "src/core/globals.h"

void drawKeyboardKey(int x, int y, int w, int h, const String& label, uint16_t borderColor, uint16_t textColor)
{
  tft.fillRoundRect(x, y, w, h, 7, TFT_DARKGREY);
  tft.drawRoundRect(x, y, w, h, 7, borderColor);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textColor, TFT_DARKGREY);
  tft.drawString(label, x + w / 2, y + h / 2 - 1, 2);
  tft.setTextDatum(TL_DATUM);
}

void drawKeyboardRow(const char* labels[], uint8_t count, int x0, int y, int w, int h, int gap)
{
  for (uint8_t i = 0; i < count; i++)
  {
    drawKeyboardKey(x0 + i * (w + gap), y, w, h, labels[i], TFT_CYAN, TFT_WHITE);
  }
}

String keyboardDisplayText()
{
  if (!editingPassword)
  {
    return keyboardBuffer;
  }

  if (keyboardBuffer.length() == 0)
  {
    return "";
  }

  String out = "";
  uint16_t len = keyboardBuffer.length();

  // แสดงตัวอักษรล่าสุดชั่วคราว 1 วินาที เพื่อให้ผู้ใช้ตรวจว่าพิมพ์ถูกหรือไม่
  bool revealLastChar = passwordRevealActive && (millis() < passwordRevealUntilMs);

  for (uint16_t i = 0; i < len; i++)
  {
    if (revealLastChar && i == len - 1)
    {
      out += keyboardBuffer.charAt(i);
    }
    else
    {
      out += "*";
    }
  }

  return out;
}

void drawKeyboardInputField(bool force)
{
  String displayText = keyboardDisplayText();
  String textToDraw = displayText;

  if (textToDraw.length() == 0)
  {
    textToDraw = editingPassword ? "Password" : "SSID";
  }
  else
  {
    textToDraw += "_";
  }

  // ถ้าข้อความเหมือนเดิม ไม่ต้องวาดซ้ำ เพื่อลดอาการกระพริบ/หน่วง
  if (!force && textToDraw == lastKeyboardFieldDrawnText)
  {
    return;
  }

  lastKeyboardFieldDrawnText = textToDraw;

  // ล้างเฉพาะช่องกรอกข้อความ ไม่ล้างทั้งหน้า
  tft.fillRect(24, 46, 432, 34, TFT_BLACK);
  tft.drawRoundRect(20, 42, 440, 42, 6, TFT_DARKGREY);

  uint16_t color = TFT_WHITE;
  if (displayText.length() == 0)
  {
    color = TFT_DARKGREY;
  }

  drawLeftTextBox(32, 52, 416, 22, textToDraw, 1, color, TFT_BLACK);
}

void serviceKeyboardPasswordReveal()
{
  if (!editingPassword) return;
  if (!passwordRevealActive) return;
  if (millis() < passwordRevealUntilMs) return;

  passwordRevealActive = false;

  // เมื่อครบ 1 วินาที ให้เปลี่ยนตัวอักษรล่าสุดกลับเป็น * โดยอัปเดตเฉพาะช่องข้อความ
  drawKeyboardInputField(false);
}

void drawPageKeyboard()
{
  String pageKey = editingPassword ? "UI_EDIT_PASSWORD_KEYBOARD" : "UI_EDIT_SSID_KEYBOARD";
  pageKey += ":" + String(keyboardMode == KEYBOARD_ABC ? "ABC" : "123");
  bool fresh = beginTftPage(pageKey, TFT_BLACK);
  if (!fresh) return;

  // เมื่อวาดหน้า keyboard ใหม่ ให้ reset cache ของช่องข้อความ
  lastKeyboardFieldDrawnText = "";

  String title = editingPassword ? "แก้ไขรหัสผ่าน WiFi" : "แก้ไขชื่อ WiFi";
  drawCenteredText(title, 8, 2, TFT_CYAN, TFT_BLACK);

  tft.drawRoundRect(20, 42, 440, 42, 6, TFT_DARKGREY);
  drawKeyboardInputField(true);

  if (keyboardMode == KEYBOARD_ABC)
  {
    const char* row1[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
    const char* row2[] = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
    const char* row3[] = {"Z", "X", "C", "V", "B", "N", "M"};
    drawKeyboardRow(row1, 10, KEY_ROW1_X, KEY_ROW1_Y, KEY_ROW1_W, KEY_ROW1_H, KEY_ROW1_GAP);
    drawKeyboardRow(row2, 9, KEY_ROW2_X, KEY_ROW2_Y, KEY_ROW2_W, KEY_ROW2_H, KEY_ROW2_GAP);
    drawKeyboardRow(row3, 7, KEY_ROW3_X, KEY_ROW3_Y, KEY_ROW3_W, KEY_ROW3_H, KEY_ROW3_GAP);
    drawKeyboardKey(KEY_MODE_BTN_X, KEY_MODE_BTN_Y, KEY_MODE_BTN_W, KEY_MODE_BTN_H, "123", TFT_ORANGE, TFT_WHITE);
  }
  else
  {
    const char* row1[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
    const char* row2[] = {"-", "_", "@", ".", "#", "*", "/", "!", "?"};
    const char* row3[] = {"+", "=", ":", ";", "(", ")", "&"};
    drawKeyboardRow(row1, 10, KEY_ROW1_X, KEY_ROW1_Y, KEY_ROW1_W, KEY_ROW1_H, KEY_ROW1_GAP);
    drawKeyboardRow(row2, 9, KEY_ROW2_X, KEY_ROW2_Y, KEY_ROW2_W, KEY_ROW2_H, KEY_ROW2_GAP);
    drawKeyboardRow(row3, 7, KEY_ROW3_X, KEY_ROW3_Y, KEY_ROW3_W, KEY_ROW3_H, KEY_ROW3_GAP);
    drawKeyboardKey(KEY_MODE_BTN_X, KEY_MODE_BTN_Y, KEY_MODE_BTN_W, KEY_MODE_BTN_H, "ABC", TFT_ORANGE, TFT_WHITE);
  }

  drawKeyboardKey(KEY_SPACE_BTN_X, KEY_SPACE_BTN_Y, KEY_SPACE_BTN_W, KEY_SPACE_BTN_H, "Space", TFT_CYAN, TFT_WHITE);
  drawKeyboardKey(KEY_DEL_BTN_X, KEY_DEL_BTN_Y, KEY_DEL_BTN_W, KEY_DEL_BTN_H, "DEL", TFT_RED, TFT_WHITE);
  drawKeyboardKey(KEY_OK_BTN_X, KEY_OK_BTN_Y, KEY_OK_BTN_W, KEY_OK_BTN_H, "OK", TFT_GREEN, TFT_WHITE);
  drawSmallFooter("กด OK เพื่อบันทึกและกลับหน้า WiFi Settings");
}

String keyboardHitKey(uint16_t x, uint16_t y)
{
  if (keyboardMode == KEYBOARD_ABC)
  {
    const char* row1[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
    const char* row2[] = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
    const char* row3[] = {"Z", "X", "C", "V", "B", "N", "M"};

    for (uint8_t i = 0; i < 10; i++) if (isInsideRect(x, y, KEY_ROW1_X + i * (KEY_ROW1_W + KEY_ROW1_GAP), KEY_ROW1_Y, KEY_ROW1_W, KEY_ROW1_H)) return String(row1[i]);
    for (uint8_t i = 0; i < 9; i++) if (isInsideRect(x, y, KEY_ROW2_X + i * (KEY_ROW2_W + KEY_ROW2_GAP), KEY_ROW2_Y, KEY_ROW2_W, KEY_ROW2_H)) return String(row2[i]);
    for (uint8_t i = 0; i < 7; i++) if (isInsideRect(x, y, KEY_ROW3_X + i * (KEY_ROW3_W + KEY_ROW3_GAP), KEY_ROW3_Y, KEY_ROW3_W, KEY_ROW3_H)) return String(row3[i]);
    if (isInsideRect(x, y, KEY_MODE_BTN_X, KEY_MODE_BTN_Y, KEY_MODE_BTN_W, KEY_MODE_BTN_H)) return "123";
  }
  else
  {
    const char* row1[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
    const char* row2[] = {"-", "_", "@", ".", "#", "*", "/", "!", "?"};
    const char* row3[] = {"+", "=", ":", ";", "(", ")", "&"};

    for (uint8_t i = 0; i < 10; i++) if (isInsideRect(x, y, KEY_ROW1_X + i * (KEY_ROW1_W + KEY_ROW1_GAP), KEY_ROW1_Y, KEY_ROW1_W, KEY_ROW1_H)) return String(row1[i]);
    for (uint8_t i = 0; i < 9; i++) if (isInsideRect(x, y, KEY_ROW2_X + i * (KEY_ROW2_W + KEY_ROW2_GAP), KEY_ROW2_Y, KEY_ROW2_W, KEY_ROW2_H)) return String(row2[i]);
    for (uint8_t i = 0; i < 7; i++) if (isInsideRect(x, y, KEY_ROW3_X + i * (KEY_ROW3_W + KEY_ROW3_GAP), KEY_ROW3_Y, KEY_ROW3_W, KEY_ROW3_H)) return String(row3[i]);
    if (isInsideRect(x, y, KEY_MODE_BTN_X, KEY_MODE_BTN_Y, KEY_MODE_BTN_W, KEY_MODE_BTN_H)) return "ABC";
  }

  if (isInsideRect(x, y, KEY_SPACE_BTN_X, KEY_SPACE_BTN_Y, KEY_SPACE_BTN_W, KEY_SPACE_BTN_H)) return "Space";
  if (isInsideRect(x, y, KEY_DEL_BTN_X, KEY_DEL_BTN_Y, KEY_DEL_BTN_W, KEY_DEL_BTN_H)) return "DEL";
  if (isInsideRect(x, y, KEY_OK_BTN_X, KEY_OK_BTN_Y, KEY_OK_BTN_W, KEY_OK_BTN_H)) return "OK";

  return "";
}

void handleKeyboardKey(const String& key)
{
  if (key.length() == 0) return;

  if (key == "ABC")
  {
    keyboardMode = KEYBOARD_ABC;
    passwordRevealActive = false;
    requestTftPageRedraw();
    drawUiScreen(true);
    return;
  }

  if (key == "123")
  {
    keyboardMode = KEYBOARD_123;
    passwordRevealActive = false;
    requestTftPageRedraw();
    drawUiScreen(true);
    return;
  }

  if (key == "Space")
  {
    keyboardBuffer += " ";
    if (editingPassword)
    {
      passwordRevealActive = true;
      passwordRevealUntilMs = millis() + 1000;
    }
    drawKeyboardInputField(false);
    return;
  }

  if (key == "DEL")
  {
    if (keyboardBuffer.length() > 0)
    {
      keyboardBuffer.remove(keyboardBuffer.length() - 1);
    }
    passwordRevealActive = false;
    drawKeyboardInputField(false);
    return;
  }

  if (key == "OK")
  {
    passwordRevealActive = false;

    if (editingPassword)
    {
      wifiPassword = keyboardBuffer;
    }
    else
    {
      wifiSsid = keyboardBuffer;
    }

    saveWifiPreferences();

    currentUiPage = UI_WIFI_SETTINGS;
    requestTftPageRedraw();
    drawUiScreen(true);
    return;
  }

  keyboardBuffer += key;

  if (editingPassword)
  {
    passwordRevealActive = true;
    passwordRevealUntilMs = millis() + 1000;
  }

  // ไม่ redraw ทั้งหน้า ให้ update เฉพาะช่องข้อความ
  drawKeyboardInputField(false);
}
