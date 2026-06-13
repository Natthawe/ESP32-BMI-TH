#ifndef BMI_LOGIC_H
#define BMI_LOGIC_H

#include <Arduino.h>
#include "states.h"

const char* stateName(BmiState s);
void resetPersonTimers();
String hiddenPasswordText(const String& password);
String sendStatusTextForDisplay(const String& s);
void changeState(BmiState newState);
bool isWeightEnoughForPerson(float weightKg);
float calculateBmi(float weightKg, float heightCm);
void printRuntimeStatus();
void printSendSummary();

#endif // BMI_LOGIC_H
