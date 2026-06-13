#include <Arduino.h>
#include <TFT_eSPI.h>

#include "src/core/config.h"
#include "src/core/globals.h"
#include "src/core/states.h"
#include "src/core/bmi_logic.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_page_wifi.h"
#include "src/services/wifi_manager.h"

// =====================================================
// Step 10.10: UI Page WiFi
// แยกหน้า WiFi ออกจาก ui_pages.cpp เพื่อให้แก้ layout WiFi ได้ง่ายขึ้น
// =====================================================


void drawPageWifiFailed()
{
  bool fresh = beginTftPage("WIFI_FAILED");
  if (fresh)
  {
    drawCenteredText("การต่อ WiFi ล้มเหลว", 55, 3, TFT_RED, TFT_BLACK);
    drawCenteredText("กรุณาตรวจสอบเครือข่าย WiFi", 110, 2, TFT_WHITE, TFT_BLACK);
    drawCenteredText("หรือกดด้านล่างเพื่อชั่งน้ำหนักแบบออฟไลน์", 150, 2, TFT_YELLOW, TFT_BLACK);
    drawOfflineTouchButton("โหมด Offline");
    drawSmallFooter("โหมดออนไลน์จำเป็นต้องใช้สัญญาณ WiFi");
  }
}

void drawPageWifiSettings()
{
  bool fresh = beginTftPage("UI_WIFI_SETTINGS", TFT_BLACK);
  if (!fresh) return;
  drawCenteredText("ตั้งค่า WiFi", 16, 3, TFT_CYAN, TFT_BLACK);
  tft.drawFastHLine(25, 55, 430, TFT_DARKGREY);
  drawLeftTextBox(35, 60, 410, 26, "SSID: " + wifiSsid, 1, TFT_WHITE, TFT_BLACK);
  drawLeftTextBox(35, 90, 410, 26, "Password: " + hiddenPasswordText(wifiPassword), 1, TFT_WHITE, TFT_BLACK);
  drawLeftTextBox(35, 120, 410, 26, "สถานะ: " + getWifiText(), 1, isOnlineMode() ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
  drawUiButton(WIFI_EDIT_SSID_BTN_X, WIFI_EDIT_SSID_BTN_Y, WIFI_EDIT_SSID_BTN_W, WIFI_EDIT_SSID_BTN_H, "แก้ไขชื่อ WiFi", TFT_CYAN, TFT_WHITE);
  drawUiButton(WIFI_EDIT_PASS_BTN_X, WIFI_EDIT_PASS_BTN_Y, WIFI_EDIT_PASS_BTN_W, WIFI_EDIT_PASS_BTN_H, "แก้ไขรหัสผ่าน", TFT_CYAN, TFT_WHITE);
  drawUiButton(WIFI_CONNECT_BTN_X, WIFI_CONNECT_BTN_Y, WIFI_CONNECT_BTN_W, WIFI_CONNECT_BTN_H, "เชื่อมต่อ WiFi", TFT_GREEN, TFT_WHITE);
  drawUiButton(WIFI_BACK_BTN_X, WIFI_BACK_BTN_Y, WIFI_BACK_BTN_W, WIFI_BACK_BTN_H, "กลับเมนูหลัก", TFT_ORANGE, TFT_WHITE);
  drawSmallFooter("แก้ไข SSID/Password ได้ด้วย On-Screen Keyboard");
}

void drawPageWifiConnectingUi()
{
  bool fresh = beginTftPage("UI_WIFI_CONNECTING", TFT_BLACK);
  if (!fresh) return;
  drawCenteredText("กำลังเชื่อมต่อ WiFi", 55, 3, TFT_CYAN, TFT_BLACK);
  drawCenteredText("SSID: " + wifiSsid, 118, 2, TFT_WHITE, TFT_BLACK);
  drawWifiConnectingStatusLine(wifiConnectStatusText);
  drawSmallFooter("กำลังลองเชื่อมต่อด้วย SSID/Password ที่บันทึกไว้");
}

void drawWifiConnectingStatusLine(const String& text)
{
  drawCenteredTextBox(text, 178, 44, 2, TFT_YELLOW, TFT_BLACK);
}

void drawPageWifiSuccessUi()
{
  bool fresh = beginTftPage("UI_WIFI_SUCCESS", TFT_BLACK);
  if (!fresh) return;
  drawCenteredText("เชื่อมต่อ WiFi สำเร็จ", 55, 3, TFT_GREEN, TFT_BLACK);
  drawCenteredText(getWifiText(), 120, 1, TFT_WHITE, TFT_BLACK);
  drawCenteredText(modeText(), 160, 1, TFT_YELLOW, TFT_BLACK);
  drawCenteredText("กำลังกลับเมนูหลัก...", 215, 2, TFT_CYAN, TFT_BLACK);
}

void drawPageWifiFailedUi()
{
  bool fresh = beginTftPage("UI_WIFI_FAILED", TFT_BLACK);
  if (!fresh) return;
  drawCenteredText("เชื่อมต่อ WiFi ไม่สำเร็จ", 55, 3, TFT_RED, TFT_BLACK);
  drawCenteredText("ระบบจะใช้งานแบบ Offline ได้", 120, 2, TFT_YELLOW, TFT_BLACK);
  drawCenteredText("ตรวจสอบ SSID / Password แล้วลองใหม่", 160, 1, TFT_WHITE, TFT_BLACK);
  drawUiButton(WIFI_FAIL_BTN_X, WIFI_FAIL_BTN_Y, WIFI_FAIL_BTN_W, WIFI_FAIL_BTN_H, "กลับเมนูหลัก", TFT_ORANGE, TFT_WHITE);
}
