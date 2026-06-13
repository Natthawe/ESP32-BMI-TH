#include "bmi_logic.h"
#include "config.h"
#include "globals.h"
#include "src/ui/components/ui_common.h"
#include "src/ui/pages/ui_page_bmi.h"


const char* stateName(BmiState s)
{
  switch (s)
  {
    case IDLE_WAIT_PERSON:      return "รอคนขึ้นเครื่อง (IDLE_WAIT_PERSON)";
    case WEIGHT_FIRST_CHECK:    return "น้ำหนักมาก่อนคน (WEIGHT_FIRST_CHECK)";
    case BLOCKED_REMOVE_OBJECT: return "แท่นไม่ว่าง กรุณาเอาของออก (BLOCKED_REMOVE_OBJECT)";
    case PERSON_DETECTED:       return "ตรวจพบบุคคล (PERSON_DETECTED)";
    case WEIGHT_STABILIZING:    return "กำลังประมวลผลน้ำหนัก (WEIGHT_STABILIZING)";
    case MEASURE_DONE:          return "ล็อกค่าวัดเสร็จสิ้น (MEASURE_DONE)";
    case SENDING:               return "กำลังบันทึกข้อมูล (SENDING)";
    case SEND_DONE:             return "แสดงผลลัพธ์เสร็จสิ้น (SEND_DONE)";
    case WAIT_PERSON_LEAVE:     return "รอคนเดินลงจากเครื่อง (WAIT_PERSON_LEAVE)";
    default:                    return "ไม่ทราบสถานะ (UNKNOWN)";
  }
}

void resetPersonTimers()
{
  personConfirmTimerRunning = false;
  personLeaveTimerRunning = false;
  weightFirstTimerRunning = false;
  personDetectStartTime = 0;
  personLeaveStartTime = 0;
  weightFirstStartTime = 0;
}

String hiddenPasswordText(const String& password)
{
  if (password.length() == 0) return "-";

  String out = "";
  for (uint16_t i = 0; i < password.length(); i++) out += "*";
  return out;
}

String sendStatusTextForDisplay(const String& s)
{
  if (s == "SUCCESS")    return "บันทึกข้อมูลลงฐานข้อมูลสำเร็จ";
  if (s == "FAILED")     return "การส่งข้อมูลล้มเหลว";
  if (s == "SENDING")    return "กำลังส่งข้อมูล...";
  if (s == "LOCAL_ONLY") return "บันทึกเฉพาะในเครื่องสำเร็จ";
  return "-";
}

void changeState(BmiState newState)
{
  if (state != newState)
  {
    state = newState;
    Serial.print("เปลี่ยนสถานะระบบ -> ");
    Serial.println(stateName(state));

    requestTftPageRedraw();
    drawTftScreen(true);
  }
}

bool isWeightEnoughForPerson(float weightKg)
{
  float minWeightKg = TEST_MODE_LOW_WEIGHT ? TEST_MIN_PERSON_WEIGHT_KG : MIN_PERSON_WEIGHT_KG;
  return weightKg >= minWeightKg;
}

float calculateBmi(float weightKg, float heightCm)
{
  if (weightKg <= 0 || heightCm <= 0) return 0.0;

  float heightM = heightCm / 100.0;
  return weightKg / (heightM * heightM);
}

void printRuntimeStatus()
{
  Serial.print("สถานะหลัก = "); Serial.print(stateName(state));
  Serial.print(" | น้ำหนัก: "); Serial.print(currentWeightKg, 3); Serial.println(" กก.");
}

void printSendSummary()
{
  Serial.println("\n========== สรุปยอดการส่งข้อมูล ==========");
  Serial.printf("ส่งผ่าน: %d | สำเร็จ: %d | ล้มเหลว: %d\n", sendCount, successCount, failCount);
}
