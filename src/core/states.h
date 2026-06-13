#ifndef STATES_H
#define STATES_H

// =====================================================
// ตัวแปรควบคุมสถานะระบบ BMI State Machine
// =====================================================
enum BmiState
{
  IDLE_WAIT_PERSON,
  WEIGHT_FIRST_CHECK,
  BLOCKED_REMOVE_OBJECT,
  PERSON_DETECTED,
  WEIGHT_STABILIZING,
  MEASURE_DONE,
  SENDING,
  SEND_DONE,
  WAIT_PERSON_LEAVE
};

// =====================================================
// ตัวแปรควบคุมหน้า UI หลัก แยกจาก State Machine ของการวัด BMI
// =====================================================
enum UiPage
{
  UI_STARTING,
  UI_THAI_TEST,
  UI_SENSOR_CHECK,
  UI_SENSOR_ERROR,
  UI_MAIN_MENU,
  UI_WIFI_SETTINGS,
  UI_EDIT_SSID,
  UI_EDIT_PASSWORD,
  UI_WIFI_CONNECTING,
  UI_WIFI_SUCCESS,
  UI_WIFI_FAILED,
  UI_BMI_MEASURE
};

enum KeyboardMode
{
  KEYBOARD_ABC,
  KEYBOARD_123
};

enum RcwlStartupCheckResult
{
  RCWL_STARTUP_OK,
  RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE,
  RCWL_STARTUP_NO_ECHO
};

#endif