#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>

#include "calibration.h"
#include "config.h"

namespace {
constexpr uint16_t CALIBRATION_MAGIC = 0x4D4D;
constexpr uint8_t CALIBRATION_VERSION = 2;

struct StoredCalibration {
  uint16_t magic;
  uint8_t version;
  float adcReference;
  float voltageFactor;
  float resistanceFactor;
  float currentZero;
  float currentFactor;
  float capacitanceFineFactor;
  float capacitanceCoarseFactor;
  uint8_t smoothing;
  int8_t encoderDirection;
  uint8_t encoderSteps;
  uint16_t longPressMs;
  uint8_t displayContrast;
  uint16_t updateIntervalMs;
  uint8_t decimalMode;
  uint8_t displayTimeoutMin;
  uint8_t keepHold;
  uint8_t resetMinMax;
  uint16_t checksum;
};

StoredCalibration values;
bool storageWasValid = false;

uint16_t checksum(const StoredCalibration &data) {
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&data);
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < sizeof(StoredCalibration) - sizeof(data.checksum); ++i) {
    crc ^= bytes[i];
    for (uint8_t bit = 0; bit < 8; ++bit)
      crc = (crc & 1U) ? (crc >> 1) ^ 0xA001U : crc >> 1;
  }
  return crc;
}

bool plausible(const StoredCalibration &data) {
  return data.magic == CALIBRATION_MAGIC &&
         data.version == CALIBRATION_VERSION &&
         data.checksum == checksum(data) &&
         isfinite(data.adcReference) && data.adcReference >= 3.0f &&
         data.adcReference <= 5.5f &&
         data.voltageFactor >= 0.5f && data.voltageFactor <= 1.5f &&
         data.resistanceFactor >= 0.5f && data.resistanceFactor <= 1.5f &&
         data.currentZero >= 1.0f && data.currentZero <= 4.0f &&
         data.currentFactor >= 0.5f && data.currentFactor <= 1.5f &&
         data.capacitanceFineFactor >= 0.5f && data.capacitanceFineFactor <= 1.5f &&
         data.capacitanceCoarseFactor >= 0.5f && data.capacitanceCoarseFactor <= 1.5f &&
         data.smoothing <= 2 && abs(data.encoderDirection) == 1 &&
         (data.encoderSteps == 1 || data.encoderSteps == 2 || data.encoderSteps == 4) &&
         data.longPressMs >= 300 && data.longPressMs <= 2000 &&
         data.updateIntervalMs >= 100 && data.updateIntervalMs <= 1000 &&
         data.decimalMode <= 2 && data.displayTimeoutMin <= 30 &&
         data.keepHold <= 1 && data.resetMinMax <= 1;
}

void loadDefaults() {
  values.magic = CALIBRATION_MAGIC;
  values.version = CALIBRATION_VERSION;
  values.adcReference = ADC_REFERENCE_VOLTAGE;
  values.voltageFactor = VOLTAGE_CORRECTION_FACTOR;
  values.resistanceFactor = RESISTANCE_CORRECTION_FACTOR;
  values.currentZero = CURRENT_ZERO_VOLTAGE;
  values.currentFactor = CURRENT_CORRECTION_FACTOR;
  values.capacitanceFineFactor = CAPACITANCE_FINE_CORRECTION_FACTOR;
  values.capacitanceCoarseFactor = CAPACITANCE_COARSE_CORRECTION_FACTOR;
  values.smoothing = 1;
  values.encoderDirection = 1;
  values.encoderSteps = 4;
  values.longPressMs = BUTTON_LONG_PRESS_MS;
  values.displayContrast = 127;
  values.updateIntervalMs = 200;
  values.decimalMode = 1;
  values.displayTimeoutMin = 0;
  values.keepHold = 0;
  values.resetMinMax = 1;
  values.checksum = checksum(values);
}
}

void beginCalibration() {
  EEPROM.get(0, values);
  storageWasValid = plausible(values);
  if (!storageWasValid) loadDefaults();
}

float calibrationValue(CalibrationField field) {
  switch (field) {
    case CalibrationField::AdcReference: return values.adcReference;
    case CalibrationField::VoltageFactor: return values.voltageFactor;
    case CalibrationField::ResistanceFactor: return values.resistanceFactor;
    case CalibrationField::CurrentZero: return values.currentZero;
    case CalibrationField::CurrentFactor: return values.currentFactor;
    case CalibrationField::CapacitanceFineFactor: return values.capacitanceFineFactor;
    default: return values.capacitanceCoarseFactor;
  }
}

void setCalibrationValue(CalibrationField field, float value) {
  switch (field) {
    case CalibrationField::AdcReference: values.adcReference = constrain(value, 3.0f, 5.5f); break;
    case CalibrationField::VoltageFactor: values.voltageFactor = constrain(value, 0.5f, 1.5f); break;
    case CalibrationField::ResistanceFactor: values.resistanceFactor = constrain(value, 0.5f, 1.5f); break;
    case CalibrationField::CurrentZero: values.currentZero = constrain(value, 1.0f, 4.0f); break;
    case CalibrationField::CurrentFactor: values.currentFactor = constrain(value, 0.5f, 1.5f); break;
    case CalibrationField::CapacitanceFineFactor: values.capacitanceFineFactor = constrain(value, 0.5f, 1.5f); break;
    case CalibrationField::CapacitanceCoarseFactor: values.capacitanceCoarseFactor = constrain(value, 0.5f, 1.5f); break;
  }
}

void saveCalibration() {
  values.magic = CALIBRATION_MAGIC;
  values.version = CALIBRATION_VERSION;
  values.checksum = checksum(values);
  EEPROM.put(0, values);
  storageWasValid = true;
}

void resetCalibration() {
  loadDefaults();
  saveCalibration();
}

int16_t settingValue(SettingField field) {
  switch (field) {
    case SettingField::Smoothing: return values.smoothing;
    case SettingField::EncoderDirection: return values.encoderDirection;
    case SettingField::EncoderSteps: return values.encoderSteps;
    case SettingField::LongPressMs: return values.longPressMs;
    case SettingField::DisplayContrast: return values.displayContrast;
    case SettingField::UpdateIntervalMs: return values.updateIntervalMs;
    case SettingField::DecimalMode: return values.decimalMode;
    case SettingField::DisplayTimeoutMin: return values.displayTimeoutMin;
    case SettingField::KeepHold: return values.keepHold;
    default: return values.resetMinMax;
  }
}

void setSettingValue(SettingField field, int16_t value) {
  switch (field) {
    case SettingField::Smoothing: values.smoothing = constrain(value, 0, 2); break;
    case SettingField::EncoderDirection: values.encoderDirection = value < 0 ? -1 : 1; break;
    case SettingField::EncoderSteps: values.encoderSteps = value <= 1 ? 1 : value <= 2 ? 2 : 4; break;
    case SettingField::LongPressMs: values.longPressMs = constrain(value, 300, 2000); break;
    case SettingField::DisplayContrast: values.displayContrast = constrain(value, 1, 255); break;
    case SettingField::UpdateIntervalMs: values.updateIntervalMs = constrain(value, 100, 1000); break;
    case SettingField::DecimalMode: values.decimalMode = constrain(value, 0, 2); break;
    case SettingField::DisplayTimeoutMin: values.displayTimeoutMin = constrain(value, 0, 30); break;
    case SettingField::KeepHold: values.keepHold = value != 0; break;
    case SettingField::ResetMinMax: values.resetMinMax = value != 0; break;
  }
}

bool calibrationStorageValid() { return storageWasValid; }
uint8_t calibrationStorageVersion() { return CALIBRATION_VERSION; }
