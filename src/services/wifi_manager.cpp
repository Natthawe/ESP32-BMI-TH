#include "wifi_manager.h"

#include <WiFi.h>

#include "src/core/config.h"
#include "src/core/globals.h"
#include "src/core/states.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_pages.h"
#include "src/ui/pages/ui_page_main.h"
#include "src/ui/pages/ui_page_wifi.h"
#include "src/hal/touch_handler.h"


bool hasWifiCredentials()
{
  // SSID ต้องไม่ว่าง ส่วน password อนุญาตให้ว่างได้สำหรับ WiFi แบบไม่มีรหัสผ่าน
  return wifiSsid.length() > 0;
}

bool isOnlineMode()
{
  // ใน Test Mode จะ preview หน้าจริงโดยไม่เชื่อม WiFi จริง
  // จึงให้ uploadToGoogleSheet เป็นตัวกำหนดสถานะ Online/Offline ของหน้า preview
  if (ENABLE_THAI_RENDERER_TEST_MODE && currentUiPage == UI_THAI_TEST)
  {
    return uploadToGoogleSheet;
  }

  return (WiFi.status() == WL_CONNECTED && uploadToGoogleSheet);
}

String modeText()
{
  if (isOnlineMode()) return "Online / ส่งข้อมูล Google Sheet";
  return "Offline / แสดงผลเฉพาะหน้าจอ";
}

String getWifiText()
{
  // mock WiFi text เฉพาะตอน preview หน้าจริงใน Test Mode
  if (ENABLE_THAI_RENDERER_TEST_MODE && currentUiPage == UI_THAI_TEST && uploadToGoogleSheet)
  {
    return wifiSsid + " / 192.168.1.111";
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    return wifiSsid + " / " + WiFi.localIP().toString();
  }
  return "ไม่ได้เชื่อมต่อ WiFi";
}

bool connectWiFiWithUi(uint32_t timeoutMs, bool showFinalPage)
{
  if (!hasWifiCredentials())
  {
    offlineMode = true;
    uploadToGoogleSheet = false;
    if (showFinalPage)
    {
      currentUiPage = UI_WIFI_FAILED;
      requestTftPageRedraw();
      drawUiScreen(true);
    }
    return false;
  }

  currentUiPage = UI_WIFI_CONNECTING;
  wifiConnectStatusText = "กำลังเริ่มเชื่อมต่อ...";
  requestTftPageRedraw();
  drawUiScreen(true);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.disconnect();
  delay(200);
  WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());

  uint32_t startMs = millis();
  int lastSecondShown = -1;

  while (WiFi.status() != WL_CONNECTED && millis() - startMs < timeoutMs)
  {
    uint32_t elapsed = millis() - startMs;
    int secondsLeft = (timeoutMs - elapsed + 999) / 1000;

    if (secondsLeft != lastSecondShown)
    {
      lastSecondShown = secondsLeft;
      wifiConnectStatusText = "เหลือเวลา " + String(secondsLeft) + " วินาที";
      drawWifiConnectingStatusLine(wifiConnectStatusText);
    }

    delay(50);
  }

  bool connected = (WiFi.status() == WL_CONNECTED);

  if (connected)
  {
    offlineMode = false;
    uploadToGoogleSheet = true;
  }
  else
  {
    offlineMode = true;
    uploadToGoogleSheet = false;
    WiFi.disconnect();
  }

  if (showFinalPage)
  {
    if (connected)
    {
      // เมื่อเชื่อมต่อ WiFi สำเร็จ ให้แสดงหน้าสำเร็จสั้น ๆ
      // แล้วกลับ MAIN MENU อัตโนมัติ โดยไม่ต้องให้ผู้ใช้กดปุ่ม
      currentUiPage = UI_WIFI_SUCCESS;
      requestTftPageRedraw();
      drawUiScreen(true);
      delay(WIFI_SUCCESS_REDIRECT_DELAY_MS);

      currentUiPage = UI_MAIN_MENU;
      requestTftPageRedraw();
      drawUiScreen(true);
    }
    else
    {
      // กรณีเชื่อมต่อไม่สำเร็จ ให้ค้างหน้า Failed
      // และให้ผู้ใช้กดปุ่ม "กลับเมนูหลัก" 
      // กรณีนี้ทำไมถึงให้ผู้ใช้กดปุ่ม เพราะผู้ใช้อาจจะต้องเปลี่ยน SSID / Password 
      currentUiPage = UI_WIFI_FAILED;
      requestTftPageRedraw();
      drawUiScreen(true);
    }
  }

  return connected;
}

void autoConnectWiFiAtStartup()
{
  if (!hasWifiCredentials())
  {
    offlineMode = true;
    uploadToGoogleSheet = false;
    drawPageStarting("ยังไม่มี SSID ที่บันทึกไว้", "เข้าเมนูหลักในโหมด Offline");
    delay(800);
    return;
  }

  bool connected = connectWiFiWithUi(WIFI_STARTUP_AUTO_TIMEOUT_MS, false);

  if (connected)
  {
    drawPageStarting("เชื่อมต่อ WiFi สำเร็จ", WiFi.localIP().toString());
  }
  else
  {
    drawPageStarting("เชื่อมต่อ WiFi ไม่สำเร็จ", "เข้าเมนูหลักในโหมด Offline");
  }

  delay(800);
}

void connectWiFiFromUi()
{
  connectWiFiWithUi(WIFI_UI_CONNECT_TIMEOUT_MS, true);
}

void connectWiFi()
{
  if (!uploadToGoogleSheet) return;
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  const uint8_t maxAttempts = 5;         
  const uint32_t connectTimeoutMs = 15000;
  const uint32_t retryCountdownMs = 5000;

  for (uint8_t attempt = 1; attempt <= maxAttempts && uploadToGoogleSheet; attempt++)
  {
    WiFi.disconnect(); delay(200);
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    uint32_t startMs = millis();
    int lastSecondShown = -1;

    while (WiFi.status() != WL_CONNECTED && millis() - startMs < connectTimeoutMs)
    {
      uint32_t elapsed = millis() - startMs;
      int secondsLeft = (connectTimeoutMs - elapsed + 999) / 1000;
      if (secondsLeft != lastSecondShown)
      {
        lastSecondShown = secondsLeft;
        drawPageStarting("กำลังเชื่อมต่อ WiFi...", "จะหมดเวลาในอีก " + String(secondsLeft) + " วินาที");
      }
      if (waitForOfflineTouchHold(TOUCH_HOLD_MS)) { enterOfflineMode("กดข้ามสายเสร็จสิ้น"); return; }
      delay(20);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      offlineMode = false; uploadToGoogleSheet = true;
      drawPageStarting("เชื่อมต่อ WiFi สำเร็จ", WiFi.localIP().toString());
      delay(900); return;
    }

    if (attempt >= maxAttempts) break;
    uint32_t retryStartMs = millis();
    int lastRetrySecondShown = -1;
    while (millis() - retryStartMs < retryCountdownMs)
    {
      int secondsLeft = (retryCountdownMs - (millis() - retryStartMs) + 999) / 1000;
      if (secondsLeft != lastRetrySecondShown)
      {
        lastRetrySecondShown = secondsLeft;
        drawPageStarting("กำลังเตรียมเชื่อมต่อใหม่...", "จะพยายามต่อใหม่ใน " + String(secondsLeft) + " วินาที");
      }
      if (waitForOfflineTouchHold(TOUCH_HOLD_MS)) { enterOfflineMode("กดทับโหมดออฟไลน์"); return; }
      delay(20);
    }
  }

  while (uploadToGoogleSheet && WiFi.status() != WL_CONNECTED)
  {
    drawPageWifiFailed();
    if (waitForOfflineTouchOrDelay(1000)) { enterOfflineMode("เลือกทำงาน Offline Mode"); return; }
  }
}
