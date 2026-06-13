#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =====================================================
// การตั้งค่าขาพินเซนเซอร์น้ำหนัก HX711
// =====================================================
#define HX711_DOUT 13
#define HX711_SCK  14

// =====================================================
// การตั้งค่าขาพินเซนเซอร์วัดระยะทาง RCWL-1655
// =====================================================
#define TRIG_PIN 25
#define ECHO_PIN 26

// =====================================================
// การตั้งค่าระบบสัมผัสจอ TFT
// =====================================================
#define TOUCH_CAL_DATA_VALUES { 300, 3644, 278, 3452, 7 }
extern uint16_t TOUCH_CAL_DATA[5];

const uint32_t TOUCH_HOLD_MS = 700;
const uint32_t STARTUP_TOUCH_SELECT_WINDOW_MS = 2500;

// ขอบเขตพื้นที่ปุ่มออฟไลน์ บนหน้าจอแนวนอนขนาด 480x320
const int OFFLINE_BTN_X = 80;
const int OFFLINE_BTN_Y = 235;
const int OFFLINE_BTN_W = 320;
const int OFFLINE_BTN_H = 55;

// ขอบเขตพื้นที่ปุ่มออนไลน์ ที่จะแสดงในหน้าพร้อมใช้งาน (READY)
const int ONLINE_BTN_X = 80;
const int ONLINE_BTN_Y = 235;
const int ONLINE_BTN_W = 320;
const int ONLINE_BTN_H = 55;

// =====================================================
// ขอบเขตปุ่มสำหรับ UI MENU / WIFI SETTINGS
// =====================================================
const int MAIN_START_BTN_X = 60;
const int MAIN_START_BTN_Y = 160;
const int MAIN_START_BTN_W = 360;
const int MAIN_START_BTN_H = 48;

const int MAIN_WIFI_BTN_X = 60;
const int MAIN_WIFI_BTN_Y = 220;
const int MAIN_WIFI_BTN_W = 360;
const int MAIN_WIFI_BTN_H = 48;

// ปุ่มกลับเมนูหลักบนหน้า READY / IDLE_WAIT_PERSON
// ใช้สำหรับออกจากโหมดวัด BMI / กลับไปแก้ WiFi / เลือกเริ่มใช้งานใหม่
const int READY_MENU_BTN_X = 110;
const int READY_MENU_BTN_Y = 234;
const int READY_MENU_BTN_W = 260;
const int READY_MENU_BTN_H = 42;

const int WIFI_EDIT_SSID_BTN_X = 30;
const int WIFI_EDIT_SSID_BTN_Y = 160;
const int WIFI_EDIT_SSID_BTN_W = 200;
const int WIFI_EDIT_SSID_BTN_H = 42;

const int WIFI_EDIT_PASS_BTN_X = 250;
const int WIFI_EDIT_PASS_BTN_Y = 160;
const int WIFI_EDIT_PASS_BTN_W = 200;
const int WIFI_EDIT_PASS_BTN_H = 42;

const int WIFI_CONNECT_BTN_X = 30;
const int WIFI_CONNECT_BTN_Y = 222;
const int WIFI_CONNECT_BTN_W = 200;
const int WIFI_CONNECT_BTN_H = 48;

const int WIFI_BACK_BTN_X = 250;
const int WIFI_BACK_BTN_Y = 222;
const int WIFI_BACK_BTN_W = 200;
const int WIFI_BACK_BTN_H = 48;

const int WIFI_FAIL_BTN_X = 140;
const int WIFI_FAIL_BTN_Y = 222;
const int WIFI_FAIL_BTN_W = 200;
const int WIFI_FAIL_BTN_H = 48;


// ปุ่มสำหรับหน้า Sensor Error
const int SENSOR_RETRY_BTN_X = 140;
const int SENSOR_RETRY_BTN_Y = 220;
const int SENSOR_RETRY_BTN_W = 200;
const int SENSOR_RETRY_BTN_H = 50;

// =====================================================
// ขอบเขตปุ่ม On-Screen Keyboard
// =====================================================
const int KEY_ROW1_X = 7;
const int KEY_ROW1_Y = 100;
const int KEY_ROW1_W = 43;
const int KEY_ROW1_H = 34;
const int KEY_ROW1_GAP = 4;

const int KEY_ROW2_X = 22;
const int KEY_ROW2_Y = 140;
const int KEY_ROW2_W = 44;
const int KEY_ROW2_H = 34;
const int KEY_ROW2_GAP = 5;

const int KEY_ROW3_X = 50;
const int KEY_ROW3_Y = 180;
const int KEY_ROW3_W = 50;
const int KEY_ROW3_H = 34;
const int KEY_ROW3_GAP = 5;

const int KEY_MODE_BTN_X = 7;
const int KEY_MODE_BTN_Y = 226;
const int KEY_MODE_BTN_W = 72;
const int KEY_MODE_BTN_H = 42;

const int KEY_SPACE_BTN_X = 84;
const int KEY_SPACE_BTN_Y = 226;
const int KEY_SPACE_BTN_W = 210;
const int KEY_SPACE_BTN_H = 42;

const int KEY_DEL_BTN_X = 299;
const int KEY_DEL_BTN_Y = 226;
const int KEY_DEL_BTN_W = 78;
const int KEY_DEL_BTN_H = 42;

const int KEY_OK_BTN_X = 382;
const int KEY_OK_BTN_Y = 226;
const int KEY_OK_BTN_W = 91;
const int KEY_OK_BTN_H = 42;

const uint32_t UI_TOUCH_SCAN_INTERVAL_MS = 90;
const uint32_t UI_TOUCH_DEBOUNCE_MS = 250;

// =====================================================
// Thai Renderer Test Mode
// -----------------------------------------------------
// true  = เปิดเครื่องแล้วเข้าโหมดทดสอบภาษาไทยทันที ก่อนตรวจ Sensor / WiFi / Main Menu
// false = กลับไปใช้งาน flow เครื่องวัด BMI ปกติ
// =====================================================
const bool ENABLE_THAI_RENDERER_TEST_MODE = false;

const int THAI_TEST_LEFT_BTN_X = 1;
const int THAI_TEST_LEFT_BTN_Y = 170;
const int THAI_TEST_LEFT_BTN_W = 25;
const int THAI_TEST_LEFT_BTN_H = 30;

const int THAI_TEST_RIGHT_BTN_X = 450;
const int THAI_TEST_RIGHT_BTN_Y = 170;
const int THAI_TEST_RIGHT_BTN_W = 25;
const int THAI_TEST_RIGHT_BTN_H = 30;


// =====================================================
// Preferences / WiFi / Google Sheet
// =====================================================
const char* const PREF_NAMESPACE = "bmi_wifi";
const char* const PREF_KEY_SSID = "ssid";
const char* const PREF_KEY_PASS = "pass";

const uint32_t WIFI_UI_CONNECT_TIMEOUT_MS = 15000;
const uint32_t WIFI_STARTUP_AUTO_TIMEOUT_MS = 8000;
const uint32_t WIFI_SUCCESS_REDIRECT_DELAY_MS = 1200;

// ลิงก์สำหรับส่งข้อมูลเข้า GOOGLE APPS SCRIPT WEB APP
const char* const GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbzxsZMF709eOkNrUcSteu7rgTSrjzlvahS_VsOBwgTOKxQPKORJ7m5-EeiM1hgsHkQ/exec";

// =====================================================
// การตั้งค่าเซนเซอร์น้ำหนัก HX711
// =====================================================
const float SCALE_FACTOR = -20067.953125;

const uint8_t READ_SAMPLES = 10;
const uint8_t TARE_SAMPLES = 20;

// =====================================================
// การตั้งค่าเกณฑ์การตรวจจับน้ำหนัก
// =====================================================
const float OBJECT_DETECT_KG = 0.10;
const float OBJECT_LEAVE_KG  = 0.05;

const float STABLE_DELTA_KG = 0.01;
const uint32_t STABLE_TIME_MS = 2000;

const uint32_t WEIGHT_READ_INTERVAL_MS = 300;

// =====================================================
// การตั้งค่าระบบป้องกันสัญญาณรบกวนผิดพลาด
// =====================================================
const float SCALE_NOT_EMPTY_KG = 0.08;

// =====================================================
// เกณฑ์น้ำหนักขั้นต่ำสำหรับเริ่มวัด BMI จริง
// =====================================================
// กันกรณีมีของวางบนแท่น เช่น กล่อง/ขวดน้ำ แล้วมีมือผ่าน sensor วัดส่วนสูง
// ถ้าน้ำหนักยังต่ำกว่าค่านี้ ระบบจะถือว่าไม่ใช่คน และจะไม่วัด/ไม่ส่ง Google Sheet
// ใช้งานจริงแนะนำ 10.0 kg หรือปรับตามกลุ่มผู้ใช้งาน
// การทดสอบชั่วคราว ให้ TEST_MODE_LOW_WEIGHT เป็น true
// ใช้งานจริง ให้ TEST_MODE_LOW_WEIGHT เป็น false
const bool TEST_MODE_LOW_WEIGHT = true;
const float MIN_PERSON_WEIGHT_KG = 10.0;
const float TEST_MIN_PERSON_WEIGHT_KG = 0.30;

const uint32_t PERSON_CONFIRM_TIME_MS = 1500;
const float WEIGHT_RISE_FROM_BASELINE_KG = 0.10;
const uint32_t PERSON_LEAVE_CONFIRM_MS = 3000;
const uint32_t WEIGHT_FIRST_GRACE_MS = 15000;

// =====================================================
// การตั้งค่าเซนเซอร์วัดระยะ RCWL-1655
// =====================================================
const float PERSON_DETECT_CM = 120.0;
const float PERSON_LEAVE_CM  = 140.0;

// ใช้เฉพาะตอนรอคนออกหลังวัดเสร็จ เพื่อยอมรับระยะพื้น/ระยะไกลกว่า MAX_VALID_DISTANCE_CM
// ไม่ใช้ในการคำนวณส่วนสูง เพราะการคำนวณยังใช้ MAX_VALID_DISTANCE_CM = 150.0 เหมือนเดิม
const float PERSON_LEAVE_RAW_MIN_CM = 10.0;
const float PERSON_LEAVE_RAW_MAX_CM = 260.0;
const uint8_t PERSON_LEAVE_RAW_SAMPLES = 3;
const uint8_t PERSON_LEAVE_RAW_MIN_OK_COUNT = 2;

const uint32_t DISTANCE_READ_INTERVAL_MS = 80;
const uint32_t DISTANCE_SAMPLE_GAP_MS = 60;
const uint32_t RCWL_PULSE_TIMEOUT_US = 35000;
const uint8_t DISTANCE_SAMPLES = 5;

const float MIN_VALID_DISTANCE_CM = 10.0;
const float MAX_VALID_DISTANCE_CM = 150.0;
const float MAX_DISTANCE_JUMP_CM = 120.0;
const uint8_t JUMP_CONFIRM_COUNT = 2;
const float JUMP_CONFIRM_TOLERANCE_CM = 20.0;
const uint8_t NO_ECHO_HOLD_COUNT = 10;

const uint32_t RCWL_DEBUG_PRINT_INTERVAL_MS = 1200;

// =====================================================
// การตั้งค่าความสูง และการคำนวณ BMI
// =====================================================
const float SENSOR_TO_FLOOR_CM = 200.0;

// ใช้สำหรับตรวจ RCWL-1655 ตอนเปิดเครื่องเท่านั้น
// ไม่ได้นำค่านี้ไปคำนวณส่วนสูง/BMI
const float RCWL_STARTUP_CHECK_MIN_CM = 150.0;
const float RCWL_STARTUP_CHECK_MAX_CM = 230.0;
const uint8_t RCWL_STARTUP_CHECK_SAMPLES = 10;
const uint8_t RCWL_STARTUP_CHECK_MIN_OK_COUNT = 3;

const float MIN_HEIGHT_CM = 80.0;
const float MAX_HEIGHT_CM = 210.0;

// =====================================================
// การตั้งค่าระบบจับระยะทางรอบสุดท้าย
// =====================================================
const uint8_t FINAL_DISTANCE_CAPTURE_ROUNDS = 5;
const uint8_t FINAL_DISTANCE_CAPTURE_SAMPLES = 9;
const uint16_t FINAL_DISTANCE_CAPTURE_DELAY_MS = 80;

// =====================================================
// การตั้งค่าระบบส่งข้อมูลเข้า Google Sheet
// =====================================================
const uint32_t HTTP_TIMEOUT_MS = 15000;

// =====================================================
// การตั้งค่าระบบอัปเดตหน้าจอ TFT
// =====================================================
const uint32_t TFT_UPDATE_INTERVAL_MS = 500;

const uint32_t MODE_TOGGLE_DEBOUNCE_MS = 1000;
const uint32_t READY_TOUCH_SCAN_INTERVAL_MS = 80;

// =====================================================
// การจูนพยัญชนะ สระ และ วรรณยุกต์
// =====================================================
const int THAI_UPPER_X_OFFSET = 0;
const int THAI_UPPER_Y_OFFSET = 0;

const int THAI_LOWER_X_OFFSET = 0;
const int THAI_LOWER_Y_OFFSET = 0;

// วรรณยุกต์เดี่ยว เช่น แท่น / ส่วน / ไม่ / ค่า
// หลักการ: วรรณยุกต์เดี่ยวควรอยู่ layer เดียวกับสระบน เช่น ไม้หันอากาศในคำว่า ชั่ง
// ค่าเริ่มต้นควรใกล้กับ THAI_UPPER_Y_OFFSET ไม่ใช่สูงเท่ากับวรรณยุกต์ซ้อน
const int THAI_TONE_X_OFFSET = 0;
const int THAI_TONE_Y_OFFSET = 0;

// วรรณยุกต์เดี่ยวที่มากับสระล่าง เช่น ปุ่ม / ทุ่ม
// ใช้ขยับวรรณยุกต์หลบตัวพยัญชนะเล็กน้อย โดยไม่กระทบคำอย่าง แท่น / ส่วน
const int THAI_TONE_AFTER_LOWER_X_OFFSET = -2;
const int THAI_TONE_AFTER_LOWER_Y_OFFSET = 0;

// วรรณยุกต์ซ้อนบนสระ เช่น ชั่ง / เครื่อง / เชื่อม / น้ำ
const int THAI_STACK_TONE_X_OFFSET = 0;
const int THAI_STACK_TONE_Y_OFFSET = -6;

// ลดช่องไฟหลังสระ ำ เช่น น้ำหนัก / สำเร็จ / คำนวณ
// ถ้ายังห่าง ให้ลดเป็น -2, -3, -4
const int THAI_SARA_AM_ADVANCE_ADJUST_X = 0;

const int THAI_SPACE_FALLBACK_W = 8;

#endif