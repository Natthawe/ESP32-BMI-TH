#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

#include "src/core/config.h"
#include "src/core/states.h"
#include "src/core/globals.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/components/ui_keyboard.h"
#include "src/ui/pages/ui_pages.h"
#include "src/ui/pages/ui_page_bmi.h"
#include "src/services/wifi_manager.h"
#include "src/core/bmi_logic.h"
#include "src/hal/touch_handler.h"
#include "src/ui/pages/ui_page_thai_test.h"


static uint32_t lastUiTouchScanMs = 0;
static uint32_t lastUiTouchActionMs = 0;
static uint32_t lastModeToggleMs = 0;
static uint32_t lastReadyTouchScanMs = 0;
static uint32_t readyTouchHoldStartMs = 0;
static bool readyTouchArmed = false;

// =====================================================
// เริ่ม flow วัด BMI จาก MAIN MENU
// =====================================================
void startBmiMeasureFromMenu()
{
  if (WiFi.status() == WL_CONNECTED && uploadToGoogleSheet)
  {
    offlineMode = false;
    uploadToGoogleSheet = true;
  }
  else
  {
    offlineMode = true;
    uploadToGoogleSheet = false;
  }

  bmiRunning = true;
  currentUiPage = UI_BMI_MEASURE;
  lastSendStatusText = "-";

  resetPersonTimers();
  changeState(IDLE_WAIT_PERSON);

  requestTftPageRedraw();
  drawTftScreen(true);
}

void enterOfflineMode(const char* reason)
{
  Serial.print("เข้าสู่โหมดออฟไลน์: ");
  Serial.println(reason);

  offlineMode = true;
  uploadToGoogleSheet = false;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  lastSendStatusText = "LOCAL_ONLY";

  requestTftPageRedraw();
  delay(700);
}

// =====================================================
// Touch handler หลักของ UI mode
// =====================================================
void handleUiTouch()
{
  uint32_t now = millis();

  if (now - lastUiTouchScanMs < UI_TOUCH_SCAN_INTERVAL_MS) return;
  lastUiTouchScanMs = now;

  uint16_t x = 0, y = 0;
  if (!readTouchPoint(&x, &y)) return;

  if (now - lastUiTouchActionMs < UI_TOUCH_DEBOUNCE_MS) return;
  lastUiTouchActionMs = now;

  if (currentUiPage == UI_THAI_TEST)
  {
    if (isInsideRect(x, y, THAI_TEST_LEFT_BTN_X, THAI_TEST_LEFT_BTN_Y, THAI_TEST_LEFT_BTN_W, THAI_TEST_LEFT_BTN_H))
    {
      thaiTestPrevPage();
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }

    if (isInsideRect(x, y, THAI_TEST_RIGHT_BTN_X, THAI_TEST_RIGHT_BTN_Y, THAI_TEST_RIGHT_BTN_W, THAI_TEST_RIGHT_BTN_H))
    {
      thaiTestNextPage();
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }

    return;
  }

  if (currentUiPage == UI_MAIN_MENU)
  {
    if (isInsideRect(x, y, MAIN_START_BTN_X, MAIN_START_BTN_Y, MAIN_START_BTN_W, MAIN_START_BTN_H))
    {
      startBmiMeasureFromMenu();
      return;
    }

    if (isInsideRect(x, y, MAIN_WIFI_BTN_X, MAIN_WIFI_BTN_Y, MAIN_WIFI_BTN_W, MAIN_WIFI_BTN_H))
    {
      currentUiPage = UI_WIFI_SETTINGS;
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }
  }
  else if (currentUiPage == UI_WIFI_SETTINGS)
  {
    if (isInsideRect(x, y, WIFI_EDIT_SSID_BTN_X, WIFI_EDIT_SSID_BTN_Y, WIFI_EDIT_SSID_BTN_W, WIFI_EDIT_SSID_BTN_H))
    {
      editingPassword = false;
      keyboardBuffer = wifiSsid;
      keyboardMode = KEYBOARD_ABC;
      passwordRevealActive = false;
      lastKeyboardFieldDrawnText = "";
      currentUiPage = UI_EDIT_SSID;
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }

    if (isInsideRect(x, y, WIFI_EDIT_PASS_BTN_X, WIFI_EDIT_PASS_BTN_Y, WIFI_EDIT_PASS_BTN_W, WIFI_EDIT_PASS_BTN_H))
    {
      editingPassword = true;
      keyboardBuffer = wifiPassword;
      keyboardMode = KEYBOARD_ABC;
      passwordRevealActive = false;
      lastKeyboardFieldDrawnText = "";
      currentUiPage = UI_EDIT_PASSWORD;
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }

    if (isInsideRect(x, y, WIFI_CONNECT_BTN_X, WIFI_CONNECT_BTN_Y, WIFI_CONNECT_BTN_W, WIFI_CONNECT_BTN_H))
    {
      connectWiFiFromUi();
      return;
    }

    if (isInsideRect(x, y, WIFI_BACK_BTN_X, WIFI_BACK_BTN_Y, WIFI_BACK_BTN_W, WIFI_BACK_BTN_H))
    {
      currentUiPage = UI_MAIN_MENU;
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }
  }
  else if (currentUiPage == UI_EDIT_SSID || currentUiPage == UI_EDIT_PASSWORD)
  {
    String key = keyboardHitKey(x, y);
    handleKeyboardKey(key);
    return;
  }
  else if (currentUiPage == UI_WIFI_FAILED)
  {
    // หน้า WiFi Failed ต้องกดปุ่มกลับเมนูหลักเท่านั้น
    if (isInsideRect(x, y, WIFI_BACK_BTN_X, WIFI_BACK_BTN_Y, WIFI_BACK_BTN_W, WIFI_BACK_BTN_H))
    {
      currentUiPage = UI_MAIN_MENU;
      requestTftPageRedraw();
      drawUiScreen(true);
      return;
    }
  }
}

// =====================================================
// ฟังก์ชันระบบสัมผัสพื้นฐาน
// =====================================================
bool readTouchPoint(uint16_t* x, uint16_t* y)
{
  return tft.getTouch(x, y);
}

bool isInsideRect(uint16_t x, uint16_t y, int rx, int ry, int rw, int rh)
{
  return (x >= rx && x <= (rx + rw) && y >= ry && y <= (ry + rh));
}

bool isOfflineTouchPressed()
{
  uint16_t x = 0, y = 0;
  if (!readTouchPoint(&x, &y)) return false;
  return isInsideRect(x, y, OFFLINE_BTN_X, OFFLINE_BTN_Y, OFFLINE_BTN_W, OFFLINE_BTN_H);
}

bool isOnlineTouchPressed()
{
  uint16_t x = 0, y = 0;
  if (!readTouchPoint(&x, &y)) return false;
  return isInsideRect(x, y, ONLINE_BTN_X, ONLINE_BTN_Y, ONLINE_BTN_W, ONLINE_BTN_H);
}

bool waitForOfflineTouchHold(uint32_t holdMs)
{
  if (!isOfflineTouchPressed()) return false;

  uint32_t startMs = millis();
  while (isOfflineTouchPressed())
  {
    if (millis() - startMs >= holdMs) return true;
    delay(20);
  }

  return false;
}

bool waitForOnlineTouchHold(uint32_t holdMs)
{
  if (!isOnlineTouchPressed()) return false;

  uint32_t startMs = millis();
  while (isOnlineTouchPressed())
  {
    if (millis() - startMs >= holdMs) return true;
    delay(20);
  }

  return false;
}

bool waitForOfflineTouchWindow(uint32_t windowMs)
{
  uint32_t startMs = millis();

  while (millis() - startMs < windowMs)
  {
    if (waitForOfflineTouchHold(TOUCH_HOLD_MS)) return true;
    delay(20);
  }

  return false;
}

bool waitForOfflineTouchOrDelay(uint32_t waitMs)
{
  uint32_t startMs = millis();

  while (millis() - startMs < waitMs)
  {
    if (waitForOfflineTouchHold(TOUCH_HOLD_MS)) return true;
    delay(20);
  }

  return false;
}

// =====================================================
// ปุ่มกลับเมนูหลักจากหน้า READY / IDLE_WAIT_PERSON
// =====================================================
void returnToMainMenuFromReady()
{
  // ออกจากโหมดวัด BMI กลับไป MAIN MENU โดยไม่ reset WiFi / Preferences
  // ใช้เมื่อต้องการแก้ WiFi, ต่อ WiFi ใหม่, หรือกลับไปเลือกเริ่มใช้งานอีกครั้ง
  Serial.println("ผู้ใช้กดกลับเมนูหลักจากหน้า READY");

  resetPersonTimers();
  state = IDLE_WAIT_PERSON;
  bmiRunning = false;
  currentUiPage = UI_MAIN_MENU;

  readyTouchHoldStartMs = 0;
  readyTouchArmed = false;
  lastSendStatusText = "-";

  requestTftPageRedraw();
}

void handleReadyTouchActions()
{
  if (state != IDLE_WAIT_PERSON)
  {
    readyTouchHoldStartMs = 0;
    readyTouchArmed = false;
    return;
  }

  uint32_t now = millis();
  if (now - lastReadyTouchScanMs < READY_TOUCH_SCAN_INTERVAL_MS) return;
  lastReadyTouchScanMs = now;

  if (now - lastModeToggleMs < MODE_TOGGLE_DEBOUNCE_MS) return;

  uint16_t x = 0, y = 0;
  if (!readTouchPoint(&x, &y)) return;

  if (isInsideRect(x, y, READY_MENU_BTN_X, READY_MENU_BTN_Y, READY_MENU_BTN_W, READY_MENU_BTN_H))
  {
    lastModeToggleMs = now;
    returnToMainMenuFromReady();
    return;
  }
}
