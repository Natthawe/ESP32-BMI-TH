#include <Arduino.h>
#include <math.h>
#include <TFT_eSPI.h>

#include "src/core/config.h"
#include "src/core/globals.h"
#include "src/core/states.h"
#include "src/core/bmi_logic.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_page_main.h"
#include "src/ui/pages/ui_page_bmi.h"
#include "src/services/wifi_manager.h"
#include "src/hal/sensors.h"


void drawResultValues(const String& title, const String& message, float weightKg, float heightCm, float bmi, uint16_t titleColor)
{
  String key = "VALUES:" + title + ":" + message + ":" + modeText();
  bool fresh = beginTftPage(key, TFT_BLACK); 
  if (fresh)
  {
    drawCenteredText(title, 20, 3, titleColor, TFT_BLACK);
    if (message.length() > 0)
    {
      drawCenteredText(message, 60, 1, TFT_LIGHTGREY, TFT_BLACK);
    }
    drawMetricLine(100, "น้ำหนัก", String(weightKg, 3) + " กก.", TFT_WHITE);
    drawMetricLine(150, "ส่วนสูง", String(heightCm, 1) + " ซม.", TFT_WHITE);
    drawMetricLine(200, "ค่า BMI", String(bmi, 2), TFT_WHITE);
    drawSmallFooter(modeText());
  }
  else
  {
    updateResultValuesOnly(weightKg, heightCm, bmi);
  }
}

void drawPageReady()
{
  String key = "READY:" + modeText();
  bool fresh = beginTftPage(key);
  if (fresh)
  {
    drawCenteredText("เครื่องวัดดัชนีมวลกาย BMI", 16, 2, TFT_LIGHTGREY, TFT_BLACK);
    tft.drawFastHLine(25, 45, 430, TFT_DARKGREY);
    drawCenteredText("พร้อมใช้งาน", 75, 4, TFT_GREEN, TFT_BLACK);
    drawCenteredText("กรุณาก้าวขึ้นมายืนบนแท่นชั่ง", 140, 2, TFT_WHITE, TFT_BLACK);
    drawCenteredText(modeText(), 194, 1, isOnlineMode() ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);

    // เพิ่มปุ่มกลับเมนูหลัก เพื่อให้ผู้ใช้เปลี่ยน WiFi / กลับไปเลือก flow ได้
    drawUiButton(READY_MENU_BTN_X, READY_MENU_BTN_Y, READY_MENU_BTN_W, READY_MENU_BTN_H,
                 "กลับเมนูหลัก", TFT_ORANGE, TFT_WHITE);

    drawSmallFooter("แตะปุ่มกลับเมนูหลัก หากต้องการเปลี่ยน WiFi/โหมด");
  }
}

void drawPageChecking()
{
  drawBasicPage("กำลังตรวจเช็ก", "กรุณายืนนิ่งๆ สักครู่", TFT_ORANGE, modeText());
}

void drawPageRemoveObject()
{
  drawBasicPage("แท่นชั่งไม่ว่าง", "กรุณานำของออกจากแท่น", TFT_RED, modeText());
}

void drawPageDetected()
{
  drawBasicPage("ตรวจพบคนยืน", "รอน้ำหนักก้าวขึ้นแท่น...", TFT_YELLOW, modeText());
}

void drawPageMeasuring(float weightKg, float heightCm, float bmi)
{
  drawResultValues("กำลังวัดค่า", "กรุณายืนตัวตรงและนิ่งๆ", weightKg, heightCm, bmi, TFT_YELLOW);
}

void drawPageCalculating()
{
  drawBasicPage("กำลังคำนวณผล", "กรุณารอสักครู่", TFT_CYAN, modeText());
}

void drawPageSending()
{
  String title = "กำลังส่งข้อมูล";
  String msg = "บันทึกลง Google Sheet...";
  if (!uploadToGoogleSheet)
  {
    title = "โหมด Offline";
    msg = "ข้ามขั้นตอนส่งข้อมูล Google Sheet";
  }
  drawResultValues(title, msg, finalWeightKg, finalHeightCm, finalBmi, TFT_CYAN);
}

void drawPageResult()
{
  String msg = sendStatusTextForDisplay(lastSendStatusText);
  drawResultValues("ผลการวัด", msg, finalWeightKg, finalHeightCm, finalBmi, TFT_GREEN);
}

void drawPagePleaseLeave()
{
  String key = "PLEASE_LEAVE:" + sendStatusTextForDisplay(lastSendStatusText);
  bool fresh = beginTftPage(key);
  if (fresh)
  {
    drawCenteredText("วัดเสร็จสิ้น", 35, 3, TFT_ORANGE, TFT_BLACK);
    drawCenteredText("กรุณาก้าวลงจากแท่นชั่งครับ", 78, 2, TFT_WHITE, TFT_BLACK);

    drawMetricLine(130, "น้ำหนัก", String(finalWeightKg, 3) + " กก.", TFT_WHITE);
    drawMetricLine(175, "ส่วนสูง", String(finalHeightCm, 1) + " ซม.", TFT_WHITE);
    drawMetricLine(220, "ค่า BMI", String(finalBmi, 2), TFT_WHITE);
    drawSmallFooter(sendStatusTextForDisplay(lastSendStatusText));
  }
  else
  {
    updatePleaseLeaveValuesOnly(finalWeightKg, finalHeightCm, finalBmi);
  }
}

void drawTftScreen(bool force)
{
  static bool firstDraw = true;
  static BmiState lastRenderedState = IDLE_WAIT_PERSON;
  static bool lastRenderedUploadMode = true;
  static String lastRenderedSendStatus = "";

  static float lastLiveWeight = -999.0;
  static float lastLiveHeight = -999.0;
  static float lastLiveBmi = -999.0;

  uint32_t now = millis();
  bool liveValuePage = (state == WEIGHT_STABILIZING || state == WAIT_PERSON_LEAVE);
  bool sendTextMatters = (state == SENDING || state == SEND_DONE || state == WAIT_PERSON_LEAVE);

  bool pageNeedsRedraw =
    firstDraw ||
    force ||
    tftFullRedrawRequested ||
    state != lastRenderedState ||
    uploadToGoogleSheet != lastRenderedUploadMode ||
    (sendTextMatters && lastSendStatusText != lastRenderedSendStatus);

  if (!pageNeedsRedraw && !liveValuePage) return;
  if (!pageNeedsRedraw && liveValuePage && (now - lastTftUpdateTime < TFT_UPDATE_INTERVAL_MS)) return;

  float displayWeight = currentWeightKg;
  float displayHeight = calculateHeightCm(currentDistanceCm);
  float displayBmi = calculateBmi(displayWeight, displayHeight);

  if (state == SENDING || state == SEND_DONE || state == WAIT_PERSON_LEAVE)
  {
    displayWeight = finalWeightKg;
    displayHeight = finalHeightCm;
    displayBmi = finalBmi;
  }

  if (pageNeedsRedraw)
  {
    switch (state)
    {
      case IDLE_WAIT_PERSON:      drawPageReady(); break;
      case WEIGHT_FIRST_CHECK:    drawPageChecking(); break;
      case BLOCKED_REMOVE_OBJECT: drawPageRemoveObject(); break;
      case PERSON_DETECTED:       drawPageDetected(); break;
      case WEIGHT_STABILIZING:    drawPageMeasuring(displayWeight, displayHeight, displayBmi); break;
      case MEASURE_DONE:          drawPageCalculating(); break;
      case SENDING:               drawPageSending(); break;
      case SEND_DONE:             drawPageResult(); break;
      case WAIT_PERSON_LEAVE:     drawPagePleaseLeave(); break;
    }

    firstDraw = false;
    tftFullRedrawRequested = false;
    lastRenderedState = state;
    lastRenderedUploadMode = uploadToGoogleSheet;
    lastRenderedSendStatus = lastSendStatusText;
    lastLiveWeight = displayWeight;
    lastLiveHeight = displayHeight;
    lastLiveBmi = displayBmi;
    lastTftUpdateTime = now;
    return;
  }

  bool liveValueChanged =
    fabs(displayWeight - lastLiveWeight) >= 0.001 ||
    fabs(displayHeight - lastLiveHeight) >= 0.1 ||
    fabs(displayBmi - lastLiveBmi) >= 0.01;

  if (!liveValueChanged) return;

  if (state == WEIGHT_STABILIZING)       updateResultValuesOnly(displayWeight, displayHeight, displayBmi);
  else if (state == WAIT_PERSON_LEAVE)   updatePleaseLeaveValuesOnly(displayWeight, displayHeight, displayBmi);

  lastLiveWeight = displayWeight;
  lastLiveHeight = displayHeight;
  lastLiveBmi = displayBmi;
  lastTftUpdateTime = now;
}

void drawTareCountdown(uint8_t secondsLeft)
{
  static int lastTareSeconds = -999;

  bool fresh = beginTftPage("TARE_COUNTDOWN", TFT_BLACK);

  if (fresh)
  {
    drawCenteredText("กำลังเตรียมชั่งน้ำหนัก", 45, 3, TFT_CYAN, TFT_BLACK);
    drawCenteredText("กรุณาอย่าเพิ่งขึ้นแท่น", 115, 2, TFT_WHITE, TFT_BLACK);
    drawSmallFooter("โปรดปล่อยแท่นชั่งให้ว่างระหว่างตั้งศูนย์");

    lastTareSeconds = -999;
  }

  // ถ้าเลขไม่เปลี่ยน ไม่ต้อง redraw ซ้ำ
  if (lastTareSeconds == (int)secondsLeft)
  {
    return;
  }

  lastTareSeconds = secondsLeft;

  String tareStatusText = "เริ่ม Tare ในอีก " + String(secondsLeft) + " วินาที";
  drawCenteredTextBox(tareStatusText, 178, 52, 2, TFT_YELLOW, TFT_BLACK);
}
