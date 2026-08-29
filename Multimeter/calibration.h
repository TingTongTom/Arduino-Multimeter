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

enum class SettingField : uint8_t {
  Smoothing,
  EncoderDirection,
  EncoderSteps,
  LongPressMs,
  DisplayContrast,
  UpdateIntervalMs,
  DecimalMode,
  DisplayTimeoutMin,
  KeepHold,
  ResetMinMax
};

void beginCalibration();
float calibrationValue(CalibrationField field);
void setCalibrationValue(CalibrationField field, float value);
void saveCalibration();
void resetCalibration();
int16_t settingValue(SettingField field);
void setSettingValue(SettingField field, int16_t value);
bool calibrationStorageValid();
uint8_t calibrationStorageVersion();
