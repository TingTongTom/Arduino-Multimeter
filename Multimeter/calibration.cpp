#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>

#include "calibration.h"
#include "config.h"

namespace {
constexpr uint16_t CALIBRATION_MAGIC = 0x4D4D;
constexpr uint8_t CALIBRATION_VERSION = 1;

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
  uint16_t checksum;
};

StoredCalibration values;

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
         data.capacitanceCoarseFactor >= 0.5f && data.capacitanceCoarseFactor <= 1.5f;
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
  values.checksum = checksum(values);
}
}

void beginCalibration() {
  EEPROM.get(0, values);
  if (!plausible(values)) loadDefaults();
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
}

void resetCalibration() {
  loadDefaults();
  saveCalibration();
}

