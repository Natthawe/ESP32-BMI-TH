#include <Arduino.h>
#include <math.h>
#include "HX711.h"
#include "src/core/config.h"
#include "src/core/states.h"
#include "src/core/globals.h"
#include "src/hal/sensors.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_pages.h"
#include "src/ui/pages/ui_page_main.h"
#include "src/ui/pages/ui_page_bmi.h"
#include "src/hal/touch_handler.h"

bool checkHX711Startup(uint32_t timeoutMs)
{
  uint32_t startMs = millis();
  while (millis() - startMs < timeoutMs)
  {
    if (scale.is_ready()) return true;
    delay(50);
  }
  return false;
}

float readDistanceRawCm()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, RCWL_PULSE_TIMEOUT_US);
  if (duration == 0) return -1.0;
  return duration * 0.0343 / 2.0;
}

RcwlStartupCheckResult checkRCWL1655Startup(float* medianOut)
{
  float floorValues[RCWL_STARTUP_CHECK_SAMPLES];
  float allValues[RCWL_STARTUP_CHECK_SAMPLES];
  uint8_t floorCount = 0;
  uint8_t anyEchoCount = 0;
  uint8_t tooCloseCount = 0;

  for (uint8_t i = 0; i < RCWL_STARTUP_CHECK_SAMPLES; i++)
  {
    float d = readDistanceRawCm();
    Serial.print("Startup RCWL sample ");
    Serial.print(i + 1);
    Serial.print(" = ");
    if (d < 0) Serial.println("NO_ECHO");
    else
    {
      Serial.print(d, 1);
      Serial.println(" cm");
      allValues[anyEchoCount] = d;
      anyEchoCount++;

      if (d >= RCWL_STARTUP_CHECK_MIN_CM && d <= RCWL_STARTUP_CHECK_MAX_CM)
      {
        floorValues[floorCount] = d;
        floorCount++;
      }
      else if (d < RCWL_STARTUP_CHECK_MIN_CM)
      {
        tooCloseCount++;
      }
    }
    delay(80);
  }

  if (floorCount >= RCWL_STARTUP_CHECK_MIN_OK_COUNT)
  {
    sortFloatArray(floorValues, floorCount);
    *medianOut = floorValues[floorCount / 2];
    return RCWL_STARTUP_OK;
  }

  if (anyEchoCount > 0)
  {
    sortFloatArray(allValues, anyEchoCount);
    *medianOut = allValues[anyEchoCount / 2];
    if (tooCloseCount >= RCWL_STARTUP_CHECK_MIN_OK_COUNT) return RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE;
    return RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE;
  }

  *medianOut = -1.0;
  return RCWL_STARTUP_NO_ECHO;
}

void waitForSensorRetry()
{
  while (true)
  {
    uint16_t x = 0, y = 0;
    if (readTouchPoint(&x, &y))
    {
      if (isInsideRect(x, y, SENSOR_RETRY_BTN_X, SENSOR_RETRY_BTN_Y, SENSOR_RETRY_BTN_W, SENSOR_RETRY_BTN_H))
      {
        delay(UI_TOUCH_DEBOUNCE_MS);
        return;
      }
    }
    delay(40);
  }
}

void runStartupSensorCheckAndTare()
{
  while (true)
  {
    hx711StartupOk = false;
    rcwlStartupResult = RCWL_STARTUP_NO_ECHO;
    rcwlStartupDistanceCm = -1.0;
    startupSensorMessage = "กำลังตรวจ HX711...";
    currentUiPage = UI_SENSOR_CHECK;
    requestTftPageRedraw();
    drawUiScreen(true);

    hx711StartupOk = checkHX711Startup(3000);
    if (!hx711StartupOk)
    {
      startupSensorMessage = "HX711 ไม่พร้อม กรุณาตรวจสายแล้วลองใหม่";
      currentUiPage = UI_SENSOR_ERROR;
      requestTftPageRedraw();
      drawUiScreen(true);
      waitForSensorRetry();
      continue;
    }

    startupSensorMessage = "กำลังตรวจ RCWL-1655...";
    requestTftPageRedraw();
    drawUiScreen(true);

    rcwlStartupResult = checkRCWL1655Startup(&rcwlStartupDistanceCm);

    if (rcwlStartupResult == RCWL_STARTUP_OK)
    {
      startupSensorMessage = "Sensor พร้อมใช้งาน กำลังตั้งศูนย์น้ำหนัก";
      requestTftPageRedraw();
      drawUiScreen(true);
      delay(600);
      break;
    }

    if (rcwlStartupResult == RCWL_STARTUP_BLOCKED_OR_TOO_CLOSE)
    {
      startupSensorMessage = "RCWL อ่านระยะผิดปกติ กรุณาตรวจว่าไม่มีวัตถุบัง sensor";
    }
    else
    {
      startupSensorMessage = "RCWL ไม่พบ Echo กรุณาตรวจสายหรือทิศทาง sensor";
    }

    currentUiPage = UI_SENSOR_ERROR;
    requestTftPageRedraw();
    drawUiScreen(true);

    waitForSensorRetry();
  }

  drawPageStarting("เซนเซอร์น้ำหนักพร้อมใช้งาน", "กรุณาอย่าเพิ่งนำสิ่งของขึ้นวางบนแท่น");
  delay(800);
  scale.set_scale(SCALE_FACTOR);

  for (int sec = 3; sec >= 1; sec--)
  {
    drawTareCountdown(sec);
    delay(1000);
  }
  scale.tare(TARE_SAMPLES);

  drawPageStarting("เซ็ตค่าศูนย์สำเร็จ", "กำลังเข้าสู่เมนูหลัก");
  delay(800);
}


// =====================================================
// RCWL / HX711 runtime sensor functions
// =====================================================
float readDistanceCm()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, RCWL_PULSE_TIMEOUT_US);
  if (duration == 0) return -1.0;

  float distanceCm = duration * 0.0343 / 2.0;
  if (distanceCm < MIN_VALID_DISTANCE_CM || distanceCm > MAX_VALID_DISTANCE_CM)
  {
    uint32_t now = millis();
    if (now - lastRcwlDebugPrintMs >= RCWL_DEBUG_PRINT_INTERVAL_MS)
    {
      lastRcwlDebugPrintMs = now;
      Serial.print("คัดแยกระยะทางที่ผิดพลาดออก: ");
      Serial.print(distanceCm, 1);
      Serial.println(" ซม.");
    }
    return -1.0;
  }
  return distanceCm;
}

void sortFloatArray(float arr[], uint8_t count)
{
  for (uint8_t i = 0; i < count - 1; i++)
  {
    for (uint8_t j = i + 1; j < count; j++)
    {
      if (arr[j] < arr[i]) { float temp = arr[i]; arr[i] = arr[j]; arr[j] = temp; }
    }
  }
}

float readDistanceMedianCm(uint8_t samples)
{
  float values[20];
  uint8_t validCount = 0;
  if (samples > 20) samples = 20;

  for (uint8_t i = 0; i < samples; i++)
  {
    float d = readDistanceCm();
    if (d >= MIN_VALID_DISTANCE_CM && d <= MAX_VALID_DISTANCE_CM) { values[validCount] = d; validCount++; }
    delay(35);
  }
  if (validCount == 0) return -1.0;

  sortFloatArray(values, validCount);
  if (validCount % 2 == 1) return values[validCount / 2];
  else return (values[(validCount / 2) - 1] + values[validCount / 2]) / 2.0;
}

void serviceDistanceSensor(uint32_t now)
{
  if (!distanceScanActive && (now - lastDistanceReadTime < DISTANCE_READ_INTERVAL_MS)) return;

  if (!distanceScanActive)
  {
    distanceScanActive = true;
    distanceSampleIndex = 0;
    distanceValidCount = 0;
    lastDistanceSampleTime = 0;
    lastDistanceReadTime = now;
  }

  if (lastDistanceSampleTime != 0 && (now - lastDistanceSampleTime < DISTANCE_SAMPLE_GAP_MS)) return;
  lastDistanceSampleTime = now;

  float d = readDistanceCm();
  if (d >= MIN_VALID_DISTANCE_CM && d <= MAX_VALID_DISTANCE_CM && distanceValidCount < 20)
  {
    distanceSampleBuffer[distanceValidCount] = d;
    distanceValidCount++;
  }
  distanceSampleIndex++;

  if (distanceSampleIndex < DISTANCE_SAMPLES) return;

  float medianDistanceCm = -1.0;
  if (distanceValidCount > 0)
  {
    sortFloatArray(distanceSampleBuffer, distanceValidCount);
    if (distanceValidCount % 2 == 1) medianDistanceCm = distanceSampleBuffer[distanceValidCount / 2];
    else medianDistanceCm = (distanceSampleBuffer[(distanceValidCount / 2) - 1] + distanceSampleBuffer[distanceValidCount / 2]) / 2.0;
  }

  currentDistanceCm = filterDistanceJump(medianDistanceCm);
  distanceScanActive = false;
}

float captureFinalDistanceCm()
{
  float values[10];
  uint8_t validCount = 0;
  Serial.println("\n-> เริ่มกระบวนการล็อกพิกัดระยะทางสุดท้าย...");

  for (uint8_t i = 0; i < FINAL_DISTANCE_CAPTURE_ROUNDS; i++)
  {
    float d = readDistanceMedianCm(FINAL_DISTANCE_CAPTURE_SAMPLES);
    Serial.print("ค่าวัดพิกัดสุดท้ายรอบที่ "); Serial.print(i + 1); Serial.print(" = ");
    if (d < 0) Serial.println("ไม่ได้รับสัญญาณสะท้อน (NO_ECHO)");
    else { Serial.print(d, 1); Serial.println(" ซม."); values[validCount] = d; validCount++; }
    delay(FINAL_DISTANCE_CAPTURE_DELAY_MS);
  }

  if (validCount == 0) return currentDistanceCm;
  sortFloatArray(values, validCount);

  float result = -1.0;
  if (validCount % 2 == 1) result = values[validCount / 2];
  else result = (values[(validCount / 2) - 1] + values[validCount / 2]) / 2.0;

  currentDistanceCm = result;
  return result;
}

float filterDistanceJump(float newDistanceCm)
{
  static float pendingDistanceCm = -1.0;
  static uint8_t pendingCount = 0;
  static uint8_t noEchoCount = 0;

  if (newDistanceCm < 0)
  {
    pendingDistanceCm = -1.0; pendingCount = 0;
    if (lastGoodDistanceCm > 0 && noEchoCount < NO_ECHO_HOLD_COUNT)
    {
      noEchoCount++;
      currentDistanceIsHeld = true;
      return lastGoodDistanceCm;
    }
    noEchoCount = 0; lastGoodDistanceCm = -1.0; currentDistanceIsHeld = false;
    return -1.0;
  }

  noEchoCount = 0; currentDistanceIsHeld = false;
  if (lastGoodDistanceCm < 0) { lastGoodDistanceCm = newDistanceCm; return newDistanceCm; }

  float diff = fabs(newDistanceCm - lastGoodDistanceCm);
  if (diff <= MAX_DISTANCE_JUMP_CM) { lastGoodDistanceCm = newDistanceCm; return newDistanceCm; }

  if (pendingDistanceCm < 0 || fabs(newDistanceCm - pendingDistanceCm) > JUMP_CONFIRM_TOLERANCE_CM)
  {
    pendingDistanceCm = newDistanceCm; pendingCount = 1;
  }
  else pendingCount++;

  if (pendingCount >= JUMP_CONFIRM_COUNT)
  {
    lastGoodDistanceCm = newDistanceCm; pendingDistanceCm = -1.0; pendingCount = 0;
    return newDistanceCm;
  }
  return lastGoodDistanceCm;
}

bool isPersonDetected()
{
  if (currentDistanceIsHeld) return false;
  if (currentDistanceCm > 0 && currentDistanceCm <= PERSON_DETECT_CM) return true;
  return false;
}

float readDistanceForPersonLeaveCm()
{
  float values[PERSON_LEAVE_RAW_SAMPLES];
  uint8_t validCount = 0;

  for (uint8_t i = 0; i < PERSON_LEAVE_RAW_SAMPLES; i++)
  {
    float d = readDistanceRawCm();
    if (d >= PERSON_LEAVE_RAW_MIN_CM && d <= PERSON_LEAVE_RAW_MAX_CM)
    {
      values[validCount] = d;
      validCount++;
    }
    delay(15);
  }

  if (validCount < PERSON_LEAVE_RAW_MIN_OK_COUNT) return -1.0;

  sortFloatArray(values, validCount);
  if (validCount % 2 == 1) return values[validCount / 2];
  return (values[(validCount / 2) - 1] + values[validCount / 2]) / 2.0;
}

bool isPersonLeave()
{
  // ในสถานะ WAIT_PERSON_LEAVE ต้องอ่านระยะพื้น/ระยะไกลได้ เช่น 164 หรือ 211 ซม.
  // จึงใช้ raw distance แยกจาก readDistanceCm() ที่จำกัด MAX_VALID_DISTANCE_CM = 150 สำหรับการวัดส่วนสูง
  if (state == WAIT_PERSON_LEAVE || state == BLOCKED_REMOVE_OBJECT || state == PERSON_DETECTED)
  {
    float leaveDistanceCm = readDistanceForPersonLeaveCm();

    if (leaveDistanceCm < 0)
    {
      // ถ้าไม่มี echo แต่ค่าน้ำหนักลงแล้ว ให้ถือว่าไม่มีคนอยู่ใต้ sensor
      return true;
    }

    if (leaveDistanceCm >= PERSON_LEAVE_CM)
    {
      return true;
    }

    return false;
  }

  if (currentDistanceCm < 0) return true;
  if (currentDistanceCm >= PERSON_LEAVE_CM) return true;
  return false;
}

float readWeightKg()
{
  if (!scale.is_ready()) return currentWeightKg;
  float kg = scale.get_units(READ_SAMPLES);
  if (kg > -0.02 && kg < 0.02)  kg = 0.0;
  if (kg < 0.0 && kg > -0.10)   kg = 0.0;
  return kg;
}

float calculateHeightCm(float distanceCm)
{
  if (distanceCm <= 0) return 0.0;
  float heightCm = SENSOR_TO_FLOOR_CM - distanceCm;
  if (heightCm < MIN_HEIGHT_CM || heightCm > MAX_HEIGHT_CM) return 0.0;
  return heightCm;
}

