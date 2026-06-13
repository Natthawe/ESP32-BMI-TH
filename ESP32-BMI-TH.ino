/**
 * @file      ESP32-BMI-TH.ino
 * @brief     ระบบตรวจวัดสุขภาพ: ชั่งน้ำหนัก, วัดส่วนสูง และคำนวณ BMI
 * @author    Natthawi Jumjai
 * @date      13 June 2026
 * @version   1.0.0 (Step 13.5 - Actual UI Pages & Test Mode)
 * * @details   โปรแกรมหลักสำหรับการทำงานของระบบ
 * - จัดการ State Machine ของระบบ (Idle, Measuring, Result)
 * - เริ่มต้นการทำงานของเซนเซอร์ (HX711 + Load Cell, RCWL-1655 + TFT LCD)
 * - ควบคุมหน้าจอ TFT UI และระบบสัมผัส
 * - เชื่อมต่อ Wi-Fi และส่งข้อมูลผ่าน Google Sheets API
 * * @hardware  - Microcontroller: ESP32
 * - Display: TFT LCD พร้อม Touch Screen
 * - Sensors: HX711 (Weight), RCWL-1655 (Height)
 * * @note      โปรเจกต์นี้ใช้ Platform Arduino IDE / จำเป็นต้องติดตั้ง Library ตามที่ระบุใน docs/
 */

#include <WiFi.h>
#include <math.h>
#include "HX711.h"
#include <FS.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>

#include "src/core/config.h"
#include "src/core/states.h"
#include "src/core/globals.h"
#include "src/core/bmi_logic.h"
#include "src/hal/sensors.h"
#include "src/hal/touch_handler.h"
#include "src/services/preferences_manager.h"
#include "src/services/wifi_manager.h"
#include "src/services/google_sheet.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/components/ui_keyboard.h"
#include "src/ui/pages/ui_pages.h"

// =====================================================
// กำหนดออบเจกต์หน้าจอ TFT
// =====================================================
TFT_eSPI tft = TFT_eSPI();

// ค่าคาลิเบรต Touch กำหนดค่าเริ่มต้นจาก src/core/config.h
uint16_t TOUCH_CAL_DATA[5] = TOUCH_CAL_DATA_VALUES;

bool offlineMode = false;
bool uploadToGoogleSheet = true;

// =====================================================
// การตั้งค่าสัญญาณ WiFi
// =====================================================
String wifiSsid = "CG";
String wifiPassword = "11111111";

// =====================================================
// ข้อความสถานะการเชื่อมต่อ WiFi สำหรับหน้า UI
// =====================================================
String wifiConnectStatusText = "กรุณารอสักครู่";

// =====================================================
// การตั้งค่าเซนเซอร์น้ำหนัก HX711
// =====================================================
HX711 scale;

uint32_t weightFirstStartTime = 0;
bool weightFirstTimerRunning = false;
uint32_t personDetectStartTime = 0;
bool personConfirmTimerRunning = false;
uint32_t personLeaveStartTime = 0;
bool personLeaveTimerRunning = false;
float baselineWeightKg = 0.0;

// =====================================================
// การตั้งค่าเซนเซอร์วัดระยะ RCWL-1655
// =====================================================
uint32_t lastRcwlDebugPrintMs = 0;
float lastGoodDistanceCm = -1.0;
bool currentDistanceIsHeld = false;

// =====================================================
// การตั้งค่าความสูง และการคำนวณ BMI
// =====================================================
uint32_t lastTftUpdateTime = 0;
String lastSendStatusText = "-";
bool tftFullRedrawRequested = true;
String activeTftPageKey = ""; 

// =====================================================
// ตัวแปรควบคุมสถานะระบบ (State Machine)
// =====================================================
BmiState state = IDLE_WAIT_PERSON;

UiPage currentUiPage = UI_STARTING;
UiPage previousUiPage = UI_STARTING;
KeyboardMode keyboardMode = KEYBOARD_ABC;
bool bmiRunning = false;
bool editingPassword = false;
String keyboardBuffer = "";

// =====================================================
// ตัวแปร On-Screen Keyboard
// - วาด keyboard ทั้งหน้าเฉพาะตอนเปิดหน้า/เปลี่ยน layout
// - ตอนพิมพ์ให้ update เฉพาะช่องข้อความด้านบน
// - หน้า Password แสดงตัวอักษรล่าสุด 1 วินาที แล้วค่อยเปลี่ยนเป็น *
// =====================================================
String lastKeyboardFieldDrawnText = "";
uint32_t passwordRevealUntilMs = 0;
bool passwordRevealActive = false;

bool hx711StartupOk = false;
RcwlStartupCheckResult rcwlStartupResult = RCWL_STARTUP_NO_ECHO;
float rcwlStartupDistanceCm = -1.0;
String startupSensorMessage = "";

// =====================================================
// ตัวแปรระบบขณะทำงาน (Runtime Variables)
// =====================================================
float currentWeightKg = 0.0;
float lastWeightKg = 0.0;
float stableStartWeightKg = 0.0;
float currentDistanceCm = -1.0;

bool distanceScanActive = false;
uint8_t distanceSampleIndex = 0;
uint8_t distanceValidCount = 0;
float distanceSampleBuffer[20];
uint32_t lastDistanceSampleTime = 0;

float finalWeightKg = 0.0;
float finalDistanceCm = -1.0;
float finalHeightCm = 0.0;
float finalBmi = 0.0;

uint32_t lastWeightReadTime = 0;
uint32_t lastDistanceReadTime = 0;
uint32_t stableStartTime = 0;
bool stableTimerRunning = false;

int sendCount = 0;    
int successCount = 0; 
int failCount = 0;    

// =====================================================
// ฟังก์ชัน SETUP หลักของระบบ
// =====================================================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  // โหลดค่า WiFi ที่เคยแก้จาก On-Screen Keyboard
  // ถ้ายังไม่เคยบันทึก จะใช้ค่า default ในโค้ด
  loadWifiPreferences();

  tft.init();
  tft.setRotation(1); 
  tft.setTouch(TOUCH_CAL_DATA);
  tft.fillScreen(TFT_BLACK);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed! Please check partition.");
  }

  // ENABLE_THAI_RENDERER_TEST_MODE
  // เปิดใช้เมื่อ "ต้องการดูข้อความไทย" ตั้งแต่หน้าแรก โดยไม่ต้องรอ Sensor/WiFi/Main Menu
  if (ENABLE_THAI_RENDERER_TEST_MODE)
  {
    Serial.println("Thai Renderer Test Mode enabled");
    bmiRunning = false;
    currentUiPage = UI_THAI_TEST;
    requestTftPageRedraw();
    drawUiScreen(true);
    return;
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  scale.begin(HX711_DOUT, HX711_SCK, true);

  // ตรวจ HX711 และ RCWL-1655 ก่อนเข้า MAIN MENU
  // ระยะ RCWL ที่อ่านในขั้นนี้ใช้ตรวจความพร้อมของ sensor เท่านั้น
  // ไม่ได้นำไปใช้คำนวณส่วนสูงหรือ BMI
  runStartupSensorCheckAndTare();

  // หลัง sensor check + Tare เสร็จ ให้ลองเชื่อมต่อ WiFi อัตโนมัติ
  // ถ้าเชื่อมต่อไม่สำเร็จ จะเข้า MAIN MENU ในโหมด Offline และผู้ใช้ยังเข้าไปตั้งค่า WiFi เองได้
  autoConnectWiFiAtStartup();

  // หลัง Tare เสร็จ ให้หยุดที่ MAIN MENU ก่อน ไม่เข้าโหมดวัดทันที
  lastSendStatusText = "-";
  resetPersonTimers();
  bmiRunning = false;
  currentUiPage = UI_MAIN_MENU;
  requestTftPageRedraw();
}

// =====================================================
// ฟังก์ชัน LOOP การทำงานวนรอบหลัก
// =====================================================
void loop()
{
  uint32_t now = millis();

  if (!bmiRunning)
  {
    if (currentUiPage == UI_EDIT_PASSWORD)
    {
      serviceKeyboardPasswordReveal();
    }

    handleUiTouch();
    drawUiScreen(false);
    return;
  }

  serviceDistanceSensor(now);

  if (now - lastWeightReadTime >= WEIGHT_READ_INTERVAL_MS)
  {
    lastWeightReadTime = now;
    lastWeightKg = currentWeightKg;
    currentWeightKg = readWeightKg();
    printRuntimeStatus();
  }

  // ให้หน้า READY รับ touch ปุ่ม "กลับเมนูหลัก" ได้
  handleReadyTouchActions();

  // ถ้าผู้ใช้กดกลับเมนูหลักแล้ว ให้ออกจาก loop รอบนี้ทันที
  // เพื่อไม่ให้ logic ของ IDLE_WAIT_PERSON ทำงานต่อในรอบเดียวกัน
  if (!bmiRunning) return;

  if (state == IDLE_WAIT_PERSON)
  {
    finalWeightKg = 0.0; finalDistanceCm = -1.0; finalHeightCm = 0.0; finalBmi = 0.0;
    stableTimerRunning = false; personLeaveTimerRunning = false; lastSendStatusText = "-";

    personConfirmTimerRunning = false;

    if (currentWeightKg > SCALE_NOT_EMPTY_KG)
    {
      resetPersonTimers();
      baselineWeightKg = 0.0;
      weightFirstStartTime = now;
      weightFirstTimerRunning = true;
      changeState(WEIGHT_FIRST_CHECK); return;
    }

    baselineWeightKg = 0.0;
  }

  else if (state == WEIGHT_FIRST_CHECK)
  {
    // ถ้าเข้ามา state นี้แต่ timer ยังไม่ถูกตั้ง ให้เริ่มนับจากเวลาปัจจุบัน
    if (!weightFirstTimerRunning || weightFirstStartTime == 0)
    {
      weightFirstStartTime = now;
      weightFirstTimerRunning = true;
    }

    if (currentWeightKg <= OBJECT_LEAVE_KG)
    {
      baselineWeightKg = 0.0; resetPersonTimers(); changeState(IDLE_WAIT_PERSON); return;
    }

    if (isPersonDetected())
    {
      if (!personConfirmTimerRunning) { personConfirmTimerRunning = true; personDetectStartTime = now; }
      if (now - personDetectStartTime >= PERSON_CONFIRM_TIME_MS)
      {
        if (!isWeightEnoughForPerson(currentWeightKg))
        {
          // พบวัตถุในระยะ RCWL แล้ว แต่น้ำหนักยังต่ำกว่าเกณฑ์คน
          // โหมดใช้งานจริง: กันกรณีกล่อง/ของวางบนแท่น + มือผ่าน sensor ไม่ให้วัดและส่งข้อมูลปลอม
          // โหมดทดสอบ: ถ้า TEST_MODE_LOW_WEIGHT = true จะผ่านด้วย TEST_MIN_PERSON_WEIGHT_KG
          resetPersonTimers(); changeState(BLOCKED_REMOVE_OBJECT); return;
        }

        resetPersonTimers(); stableStartWeightKg = currentWeightKg;
        stableStartTime = now; stableTimerRunning = true;
        changeState(WEIGHT_STABILIZING); return;
      }
    }
    else personConfirmTimerRunning = false;

    if (now - weightFirstStartTime >= WEIGHT_FIRST_GRACE_MS)
    {
      resetPersonTimers(); changeState(BLOCKED_REMOVE_OBJECT); return;
    }
  }

  else if (state == BLOCKED_REMOVE_OBJECT)
  {
    if (currentWeightKg <= OBJECT_LEAVE_KG && isPersonLeave())
    {
      resetPersonTimers(); changeState(IDLE_WAIT_PERSON);
    }
  }

  else if (state == PERSON_DETECTED)
  {
    if (isPersonLeave() && currentWeightKg <= OBJECT_LEAVE_KG)
    {
      if (!personLeaveTimerRunning) { personLeaveTimerRunning = true; personLeaveStartTime = millis(); }
      if (millis() - personLeaveStartTime >= PERSON_LEAVE_CONFIRM_MS)
      {
        resetPersonTimers(); changeState(IDLE_WAIT_PERSON);
      }
    }
    else
    {
      personLeaveTimerRunning = false;
      if ((currentWeightKg - baselineWeightKg) >= WEIGHT_RISE_FROM_BASELINE_KG)
      {
        stableStartWeightKg = currentWeightKg; stableStartTime = millis(); stableTimerRunning = true;
        changeState(WEIGHT_STABILIZING);
      }
    }
  }

  else if (state == WEIGHT_STABILIZING)
  {
    if (currentWeightKg <= OBJECT_LEAVE_KG)
    {
      if (!personLeaveTimerRunning) { personLeaveTimerRunning = true; personLeaveStartTime = millis(); }
      if (millis() - personLeaveStartTime >= PERSON_LEAVE_CONFIRM_MS)
      {
        stableTimerRunning = false; resetPersonTimers(); changeState(IDLE_WAIT_PERSON); return;
      }
    }
    else personLeaveTimerRunning = false;

    if (!isWeightEnoughForPerson(currentWeightKg))
    {
      stableTimerRunning = false; resetPersonTimers(); changeState(BLOCKED_REMOVE_OBJECT); return;
    }

    if (fabs(currentWeightKg - stableStartWeightKg) > STABLE_DELTA_KG)
    {
      stableStartWeightKg = currentWeightKg; stableStartTime = millis(); stableTimerRunning = true;
    }
    if (stableTimerRunning && (millis() - stableStartTime >= STABLE_TIME_MS))
    {
      finalWeightKg = currentWeightKg; changeState(MEASURE_DONE);
    }
  }

  else if (state == MEASURE_DONE)
  {
    if (!isWeightEnoughForPerson(finalWeightKg))
    {
      // สุดท้ายก่อนคำนวณ/ส่งข้อมูล: น้ำหนักต้องถึงเกณฑ์คนจริง
      resetPersonTimers(); changeState(BLOCKED_REMOVE_OBJECT); return;
    }

    finalDistanceCm = captureFinalDistanceCm();
    finalHeightCm   = calculateHeightCm(finalDistanceCm);
    finalBmi        = calculateBmi(finalWeightKg, finalHeightCm);
    changeState(SENDING);
  }

  else if (state == SENDING)
  {
    bool ok = sendToGoogleSheet(finalWeightKg, finalDistanceCm, finalHeightCm, finalBmi, "MEASURE_DONE");
    sendCount++;
    if (ok) successCount++; else failCount++;
    printSendSummary();
    changeState(SEND_DONE);
  }

  else if (state == SEND_DONE)
  {
    resetPersonTimers(); changeState(WAIT_PERSON_LEAVE);
  }

  else if (state == WAIT_PERSON_LEAVE)
  {
    if (isPersonLeave() && currentWeightKg <= OBJECT_LEAVE_KG)
    {
      resetPersonTimers(); changeState(IDLE_WAIT_PERSON);
    }
  }
}
