#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>

#include "src/core/config.h"
#include "src/core/globals.h"
#include "src/core/states.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_page_main.h"
#include "src/services/wifi_manager.h"


void drawPageStarting(const String& line1, const String& line2)
{
  static String lastLine1 = "";
  static String lastLine2 = "";

  bool fresh = beginTftPage("STARTING");

  // เปิด/ปิดกรอบ debug
  const bool SHOW_TEXT_DEBUG_BOX = false;

  if (fresh)
  {
    lastLine1 = "";
    lastLine2 = "";

    // -------------------------------
    // กรอบตำแหน่ง Title
    // -------------------------------
    if (SHOW_TEXT_DEBUG_BOX)
    {
      tft.drawRect(0, 20, 480, 60, TFT_BLUE);
      tft.drawFastHLine(0, 28, 480, TFT_DARKGREY);   // เส้น y ที่ส่งเข้า drawCenteredText
    }

    drawCenteredText("ระบบเครื่องวัด BMI", 28, 3, TFT_CYAN, TFT_BLACK);

    // -------------------------------
    // กรอบข้อความแนะนำ
    // -------------------------------
    if (SHOW_TEXT_DEBUG_BOX)
    {
      tft.drawRect(0, 198, 480, 34, TFT_ORANGE);
      tft.drawFastHLine(0, 205, 480, TFT_DARKGREY);  // เส้น y ที่ส่งเข้า drawCenteredText
    }

    // drawCenteredText("ระบบจะเข้าสู่เมนูหลักหลังเตรียมระบบเสร็จ", 205, 1, TFT_YELLOW, TFT_BLACK);

    // -------------------------------
    // กรอบปุ่ม Local Only
    // -------------------------------
    if (SHOW_TEXT_DEBUG_BOX)
    {
      tft.drawRect(OFFLINE_BTN_X, OFFLINE_BTN_Y, OFFLINE_BTN_W, OFFLINE_BTN_H, TFT_RED);
      tft.drawFastHLine(OFFLINE_BTN_X, OFFLINE_BTN_Y + (OFFLINE_BTN_H / 2), OFFLINE_BTN_W, TFT_DARKGREY);
      tft.drawFastVLine(OFFLINE_BTN_X + (OFFLINE_BTN_W / 2), OFFLINE_BTN_Y, OFFLINE_BTN_H, TFT_DARKGREY);
    }

    drawSmallFooter("กรุณารอสักครู่");
  }

  if (fresh || line1 != lastLine1)
  {
    // ล้างพื้นที่ข้อความ line1
    tft.fillRect(0, 88, 480, 72, TFT_BLACK);

    // -------------------------------
    // กรอบ line1
    // -------------------------------
    if (SHOW_TEXT_DEBUG_BOX)
    {
      tft.drawRect(0, 88, 480, 72, TFT_GREEN);
      tft.drawFastHLine(0, 92, 480, TFT_DARKGREY);   // เส้น y ที่ส่งเข้า drawCenteredTextBox
      tft.drawFastHLine(0, 92 + 32, 480, TFT_DARKGREY); // เส้นกลางโดยประมาณ
    }

    drawCenteredTextBox(line1, 92, 64, 2, TFT_WHITE, TFT_BLACK);
    lastLine1 = line1;
  }

  if (fresh || line2 != lastLine2)
  {
    // ล้างพื้นที่ข้อความ line2
    tft.fillRect(0, 156, 480, 72, TFT_BLACK);

    // -------------------------------
    // กรอบ line2
    // -------------------------------
    if (SHOW_TEXT_DEBUG_BOX)
    {
      tft.drawRect(0, 156, 480, 72, TFT_MAGENTA);
      tft.drawFastHLine(0, 160, 480, TFT_DARKGREY);  // เส้น y ที่ส่งเข้า drawCenteredTextBox
      tft.drawFastHLine(0, 160 + 32, 480, TFT_DARKGREY); // เส้นกลางโดยประมาณ
    }

    drawCenteredTextBox(line2, 160, 64, 2, TFT_LIGHTGREY, TFT_BLACK);
    lastLine2 = line2;
  }
}

void drawBasicPage(const String& title, const String& message, uint16_t titleColor, const String& footer)
{
  String key = "BASIC:" + title + ":" + footer;
  bool fresh = beginTftPage(key);
  if (fresh)
  {
    drawCenteredText("เครื่องวัดดัชนีมวลกาย BMI", 16, 2, TFT_LIGHTGREY, TFT_BLACK);
    tft.drawFastHLine(25, 45, 430, TFT_DARKGREY);
    drawCenteredText(title, 85, 4, titleColor, TFT_BLACK);

    if (message.length() > 0)
    {
      drawCenteredText(message, 155, 2, TFT_WHITE, TFT_BLACK);
    }
    if (footer.length() > 0)
    {
      drawSmallFooter(footer);
    }
  }
}

void drawPageSensorCheck()
{
  bool fresh = beginTftPage("UI_SENSOR_CHECK:" + startupSensorMessage, TFT_BLACK);
  if (!fresh) return;

  drawCenteredText("ตรวจสอบอุปกรณ์", 16, 3, TFT_CYAN, TFT_BLACK);
  tft.drawFastHLine(25, 55, 430, TFT_DARKGREY);

  drawLeftTextBox(35, 82, 410, 28, "HX711 / Load Cell: " + String(hx711StartupOk ? "พร้อมใช้งาน" : "กำลังตรวจสอบ..."), 1, hx711StartupOk ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);

  String rcwlLine = "RCWL-1655: กำลังตรวจสอบ...";
  uint16_t rcwlColor = TFT_YELLOW;
  if (rcwlStartupResult == RCWL_STARTUP_OK)
  {
    rcwlLine = "RCWL-1655: พร้อมใช้งาน / ระยะ " + String(rcwlStartupDistanceCm, 1) + " ซม.";
    rcwlColor = TFT_GREEN;
  }
  else if (rcwlStartupResult == RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE)
  {
    rcwlLine = "RCWL-1655: ระยะผิดปกติ " + String(rcwlStartupDistanceCm, 1) + " ซม.";
    rcwlColor = TFT_ORANGE;
  }
  else if (rcwlStartupResult == RCWL_STARTUP_NO_ECHO)
  {
    rcwlLine = "RCWL-1655: ยังไม่พบ Echo";
    rcwlColor = TFT_RED;
  }

  drawLeftTextBox(35, 122, 410, 28, rcwlLine, 1, rcwlColor, TFT_BLACK);
  drawCenteredText(startupSensorMessage, 175, 1, TFT_WHITE, TFT_BLACK);
  drawSmallFooter("ค่าระยะ RCWL หน้านี้ใช้ตรวจ sensor เท่านั้น");
}

void drawPageSensorError()
{
  bool fresh = beginTftPage("UI_SENSOR_ERROR:" + startupSensorMessage, TFT_BLACK);
  if (!fresh) return;

  drawCenteredText("ตรวจพบปัญหา Sensor", 16, 3, TFT_RED, TFT_BLACK);
  tft.drawFastHLine(25, 55, 430, TFT_DARKGREY);

  uint16_t hxColor = hx711StartupOk ? TFT_GREEN : TFT_RED;
  drawLeftTextBox(35, 78, 410, 28, "HX711: " + String(hx711StartupOk ? "พร้อมใช้งาน" : "ไม่พร้อม / ตรวจสาย DOUT-SCK"), 1, hxColor, TFT_BLACK);

  String rcwlText = "RCWL-1655: ยังไม่พบ Echo";
  uint16_t rcwlColor = TFT_RED;
  if (rcwlStartupResult == RCWL_STARTUP_OK)
  {
    rcwlText = "RCWL-1655: พร้อมใช้งาน";
    rcwlColor = TFT_GREEN;
  }
  else if (rcwlStartupResult == RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE)
  {
    rcwlText = "RCWL-1655: ระยะผิดปกติ " + String(rcwlStartupDistanceCm, 1) + " ซม.";
    rcwlColor = TFT_ORANGE;
  }
  drawLeftTextBox(35, 116, 410, 28, rcwlText, 1, rcwlColor, TFT_BLACK);

  drawCenteredText(startupSensorMessage, 160, 1, TFT_YELLOW, TFT_BLACK);
  drawUiButton(SENSOR_RETRY_BTN_X, SENSOR_RETRY_BTN_Y, SENSOR_RETRY_BTN_W, SENSOR_RETRY_BTN_H, "ลองใหม่", TFT_CYAN, TFT_WHITE);
  drawSmallFooter("ต้องให้ HX711 และ RCWL-1655 พร้อมก่อน จึงจะเข้าเมนูหลักได้");
}

void drawPageMainMenu()
{
  bool fresh = beginTftPage("UI_MAIN_MENU", TFT_BLACK);
  if (!fresh) return;
  drawCenteredText("เครื่องวัดดัชนีมวลกาย BMI", 16, 2, TFT_CYAN, TFT_BLACK);
  tft.drawFastHLine(25, 45, 430, TFT_DARKGREY);
  drawLeftTextBox(35, 68, 410, 28, "WiFi: " + getWifiText(), 1, TFT_WHITE, TFT_BLACK);
  drawLeftTextBox(35, 103, 410, 28, "โหมด: " + modeText(), 1, isOnlineMode() ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
  drawUiButton(MAIN_START_BTN_X, MAIN_START_BTN_Y, MAIN_START_BTN_W, MAIN_START_BTN_H, "เริ่มใช้งานเครื่องวัด BMI", TFT_GREEN, TFT_WHITE);
  drawUiButton(MAIN_WIFI_BTN_X, MAIN_WIFI_BTN_Y, MAIN_WIFI_BTN_W, MAIN_WIFI_BTN_H, "ตั้งค่า WiFi", TFT_CYAN, TFT_WHITE);
  drawSmallFooter("แตะปุ่มเพื่อเลือกการทำงาน");
}

