#pragma once

#include <Arduino.h>

enum class CalibrationField : uint8_t {
  AdcReference,
  VoltageFactor,
  ResistanceFactor,
  CurrentZero,
  CurrentFactor,
  CapacitanceFineFactor,
  CapacitanceCoarseFactor
};

void beginCalibration();
float calibrationValue(CalibrationField field);
void setCalibrationValue(CalibrationField field, float value);
void saveCalibration();
void resetCalibration();

