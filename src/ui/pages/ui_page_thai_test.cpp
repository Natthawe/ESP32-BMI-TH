#include <Arduino.h>
#include <TFT_eSPI.h>
#include <LittleFS.h>

#include "src/core/config.h"
#include "src/core/globals.h"
#include "src/core/states.h"
#include "src/core/bmi_logic.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/components/ui_keyboard.h"
#include "src/ui/pages/ui_page_main.h"
#include "src/ui/pages/ui_page_bmi.h"
#include "src/ui/pages/ui_page_wifi.h"
#include "src/ui/pages/ui_page_thai_test.h"
#include "src/services/wifi_manager.h"


static int gThaiTestPage = 0;
static const int THAI_TEST_PAGE_COUNT = 23;

int thaiTestPageIndex()
{
  return gThaiTestPage;
}

int thaiTestPageCount()
{
  return THAI_TEST_PAGE_COUNT;
}

void thaiTestNextPage()
{
  gThaiTestPage++;
  if (gThaiTestPage >= THAI_TEST_PAGE_COUNT) gThaiTestPage = 0;
}

void thaiTestPrevPage()
{
  gThaiTestPage--;
  if (gThaiTestPage < 0) gThaiTestPage = THAI_TEST_PAGE_COUNT - 1;
}

static void applyCommonPreviewValues()
{
  wifiSsid = "CG";
  wifiPassword = "11111111";
  wifiConnectStatusText = "เหลือเวลา 8 วินาที";

  currentWeightKg = 0.325;
  currentDistanceCm = 34.4;
  finalWeightKg = 0.325;
  finalDistanceCm = 34.4;
  finalHeightCm = 165.6;
  finalBmi = 0.12;

  hx711StartupOk = true;
  rcwlStartupResult = RCWL_STARTUP_OK;
  rcwlStartupDistanceCm = 200.0;
  startupSensorMessage = "Sensor พร้อมใช้งาน";

  lastSendStatusText = "SUCCESS";
}

static void drawActualPageOverlay()
{
  // ปุ่มลูกศรอยู่ด้านข้าง เพื่อไม่ทับ footer อาจจะทับปุ่ม On-Screen Keyboard เล็กน้อย
  // drawUiButton(THAI_TEST_LEFT_BTN_X, THAI_TEST_LEFT_BTN_Y,
  //              THAI_TEST_LEFT_BTN_W, THAI_TEST_LEFT_BTN_H,
  //              "<", TFT_DARKGREY, TFT_WHITE);

  // drawUiButton(THAI_TEST_RIGHT_BTN_X, THAI_TEST_RIGHT_BTN_Y,
  //              THAI_TEST_RIGHT_BTN_W, THAI_TEST_RIGHT_BTN_H,
  //              ">", TFT_DARKGREY, TFT_WHITE);

  drawUiButton(THAI_TEST_LEFT_BTN_X, THAI_TEST_LEFT_BTN_Y,
               THAI_TEST_LEFT_BTN_W, THAI_TEST_LEFT_BTN_H,
               "<", TFT_BLACK, TFT_BLACK);

  drawUiButton(THAI_TEST_RIGHT_BTN_X, THAI_TEST_RIGHT_BTN_Y,
               THAI_TEST_RIGHT_BTN_W, THAI_TEST_RIGHT_BTN_H,
               ">", TFT_BLACK, TFT_BLACK);

  // แสดงเลขหน้าบริเวณมุมขวาบน เพื่อให้รู้ว่ากำลังดู page ไหน
  // tft.loadFont("Kanit-Medium-18", LittleFS);
  // String pageText = String(gThaiTestPage + 1) + "/" + String(THAI_TEST_PAGE_COUNT);
  // int w = thaiTextWidthFixed(pageText);
  // drawThaiStringFixed(pageText, 474 - w, 8, 1, TFT_DARKGREY, TFT_BLACK);
  // tft.unloadFont();
}

static void previewStartingBoot()
{
  drawPageStarting("กำลังเริ่มระบบ", "เตรียมหน้าจอและฟอนต์ภาษาไทย");
}

static void previewStartingSensor()
{
  drawPageStarting("กำลังตรวจอุปกรณ์", "HX711 / RCWL-1655");
}

static void previewTareCountdown()
{
  drawTareCountdown(3);
}

static void previewSensorChecking()
{
  hx711StartupOk = false;
  rcwlStartupResult = RCWL_STARTUP_NO_ECHO;
  rcwlStartupDistanceCm = -1.0;
  startupSensorMessage = "กำลังตรวจสอบ sensor...";
  drawPageSensorCheck();
}

static void previewSensorOk()
{
  hx711StartupOk = true;
  rcwlStartupResult = RCWL_STARTUP_OK;
  rcwlStartupDistanceCm = 200.0;
  startupSensorMessage = "Sensor พร้อมใช้งาน";
  drawPageSensorCheck();
}

static void previewSensorError()
{
  hx711StartupOk = false;
  rcwlStartupResult = RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE;
  rcwlStartupDistanceCm = 42.5;
  startupSensorMessage = "กรุณาตรวจสาย sensor และพื้นที่ใต้หัววัด";
  drawPageSensorError();
}

static void previewMainMenuOffline()
{
  offlineMode = true;
  uploadToGoogleSheet = false;
  drawPageMainMenu();
}

static void previewWifiSettings()
{
  offlineMode = true;
  uploadToGoogleSheet = false;
  drawPageWifiSettings();
}

static void previewWifiConnecting()
{
  wifiConnectStatusText = "เหลือเวลา 8 วินาที";
  drawPageWifiConnectingUi();
}

static void previewWifiSuccess()
{
  offlineMode = false;
  uploadToGoogleSheet = true;
  drawPageWifiSuccessUi();
}

static void previewWifiFailed()
{
  offlineMode = true;
  uploadToGoogleSheet = false;
  drawPageWifiFailedUi();
}

static void previewKeyboardSsid()
{
  editingPassword = false;
  keyboardMode = KEYBOARD_ABC;
  keyboardBuffer = "CG";
  passwordRevealActive = false;
  lastKeyboardFieldDrawnText = "";
  drawPageKeyboard();
}

static void previewKeyboardPassword()
{
  editingPassword = true;
  keyboardMode = KEYBOARD_123;
  keyboardBuffer = "11111111";
  passwordRevealActive = true;
  passwordRevealUntilMs = millis() + 2000;
  lastKeyboardFieldDrawnText = "";
  drawPageKeyboard();
}

static void previewReady()
{
  offlineMode = true;
  uploadToGoogleSheet = false;
  drawPageReady();
}

static void previewChecking()
{
  drawPageChecking();
}

static void previewRemoveObject()
{
  drawPageRemoveObject();
}

static void previewDetected()
{
  drawPageDetected();
}

static void previewMeasuring()
{
  drawPageMeasuring(0.325, 165.6, 0.12);
}

static void previewCalculating()
{
  drawPageCalculating();
}

static void previewSendingOnline()
{
  offlineMode = false;
  uploadToGoogleSheet = true;
  finalWeightKg = 0.325;
  finalHeightCm = 165.6;
  finalBmi = 0.12;
  drawPageSending();
}

static void previewSendingOffline()
{
  offlineMode = true;
  uploadToGoogleSheet = false;
  finalWeightKg = 0.325;
  finalHeightCm = 165.6;
  finalBmi = 0.12;
  drawPageSending();
}

static void previewResult()
{
  lastSendStatusText = "SUCCESS";
  finalWeightKg = 0.325;
  finalHeightCm = 165.6;
  finalBmi = 0.12;
  drawPageResult();
}

static void previewPleaseLeave()
{
  lastSendStatusText = "SUCCESS";
  finalWeightKg = 0.325;
  finalHeightCm = 165.6;
  finalBmi = 0.12;
  drawPagePleaseLeave();
}

void drawPageThaiTest()
{
  // ให้ทุกครั้งที่เปลี่ยน test page เป็น full redraw ของหน้าจริง
  applyCommonPreviewValues();

  switch (gThaiTestPage)
  {
    case 0:  previewStartingBoot(); break;
    case 1:  previewStartingSensor(); break;
    case 2:  previewTareCountdown(); break;
    case 3:  previewSensorChecking(); break;
    case 4:  previewSensorOk(); break;
    case 5:  previewSensorError(); break;
    case 6:  previewMainMenuOffline(); break;
    case 7:  previewWifiSettings(); break;
    case 8:  previewWifiConnecting(); break;
    case 9:  previewWifiSuccess(); break;
    case 10: previewWifiFailed(); break;
    case 11: previewKeyboardSsid(); break;
    case 12: previewKeyboardPassword(); break;
    case 13: previewReady(); break;
    case 14: previewChecking(); break;
    case 15: previewRemoveObject(); break;
    case 16: previewDetected(); break;
    case 17: previewMeasuring(); break;
    case 18: previewCalculating(); break;
    case 19: previewSendingOnline(); break;
    case 20: previewSendingOffline(); break;
    case 21: previewResult(); break;
    case 22: previewPleaseLeave(); break;
    default: previewStartingBoot(); break;
  }

  drawActualPageOverlay();
}
